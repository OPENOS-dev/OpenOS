#!/usr/bin/env python3
# Copyright 2024 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Build and bundle an individual AP image.

This is the entry point for the AP firmware recipe.
It gets invoked by chromite/api/controller/firmware.py.
"""

import argparse
import glob
import multiprocessing
import os
import pathlib
import re
import shlex
import subprocess
import sys

# pylint: disable=import-error
from google.protobuf import json_format

# pylint: disable=wrong-import-order
from chromite.api.gen_sdk.chromite.api import firmware_pb2
from chromite.lib import chromeos_version
from chromite.lib import chroot_lib
from chromite.lib import sysroot_lib
from chromite.service import artifacts


DEFAULT_BUNDLE_DIRECTORY = "/tmp/artifact_bundles"
DEFAULT_BUNDLE_METADATA_FILE = "/tmp/artifact_bundle_metadata"

ROOT_DIR = pathlib.Path(__file__).parent.parent.parent.parent.parent.resolve()


def build(_opts):
    """Builds one AP firmware targets.

    Reads FW_NAME from the adjacent firmware_target script,
    and builds that firmware.
    """
    env = os.environ.copy()
    if _opts.avb_enabled:
        env["USE"] = "avb"

    firmware_targets = _opts.firmware_targets.split(",")

    for build_target in firmware_targets:
        cmd = [
            "setup_board",
            "-b",
            build_target,
            "--force",
        ]
        log_cmd(cmd, env=env)
        subprocess.run(
            cmd,
            env=env,
            check=True,
        )

    failed_targets = []
    for build_target in firmware_targets:
        cmd = [
            "cros",
            "ap",
            "build",
            f"--build-target={build_target}",
        ]
        log_cmd(cmd, env=env)
        try:
            subprocess.run(
                cmd,
                env=env,
                check=True,
            )
        except subprocess.CalledProcessError as e:
            print(e)
            failed_targets.append(build_target)
    if failed_targets:
        print(f"Some targets failed: {failed_targets}")
        return 1
    return 0


def log_cmd(cmd, env=None, cwd=None):
    """Log subprocess command."""
    if cwd:
        print(f"cd {cwd};", end=" ")
    if env is not None:
        print("env", end=" ")
        [  # pylint:disable=expression-not-assigned
            print(key + "=" + shlex.quote(str(value)), end=" ")
            for key, value in env.items()
        ]
    print(" ".join(shlex.quote(str(x)) for x in cmd))
    sys.stdout.flush()


def find_checkout():
    """Find the path to the base of the checkout (e.g., ~/chromiumos)."""
    for path in pathlib.Path(__file__).resolve().parents:
        if (path / ".repo").is_dir():
            return path
    raise FileNotFoundError("Unable to locate the root of the checkout")


def get_version():
    """Determine the current chroot version."""
    ver = chromeos_version.VersionInfo.from_repo(source_repo=find_checkout())
    if ver:
        return ver.VersionString()
    return None


def test(_opts):
    """Runs all of the unit tests for AP firmware."""
    env = os.environ.copy()
    firmware_targets = _opts.firmware_targets.split(",")
    for build_target in firmware_targets:
        cmd = [
            "cros",
            "build-packages",
            "-v",
            "--board",
            build_target,
            "chromeos-base/ec-utils",
        ]
        log_cmd(cmd, env=env)
        subprocess.run(
            cmd,
            env=env,
            check=True,
        )
        cmd = [
            "cros_run_unit_tests",
            "--emerge-verbose",
            "--board",
            build_target,
            "--packages",
            "sys-boot/chromeos-bootimage "
            "sys-boot/coreboot "
            "sys-boot/depthcharge "
            "chromeos-base/vboot_reference",
            "--filter-only-cros-workon",
            "--no-testable-packages-ok",
        ]
        log_cmd(cmd, env=env)
        subprocess.run(cmd, check=True, env=env, stdin=subprocess.DEVNULL)
    return 0


def get_bundle_dir(opts):
    """Get the directory for the bundle from opts or use the default.

    Also create the directory if it doesn't exist.
    """
    if opts.output_dir:
        bundle_dir = opts.output_dir
    else:
        bundle_dir = DEFAULT_BUNDLE_DIRECTORY
    bundle_dir = pathlib.Path(bundle_dir)
    if not os.path.isdir(bundle_dir):
        os.mkdir(bundle_dir)
    return bundle_dir


def write_metadata(opts, info):
    """Write the metadata about the bundle."""
    bundle_metadata_file = (
        opts.metadata if opts.metadata else DEFAULT_BUNDLE_METADATA_FILE
    )
    with open(bundle_metadata_file, "w", encoding="utf-8") as file:
        file.write(json_format.MessageToJson(info))


def bundle(opts):
    """Bundles the artifacts from each target into its own tarball."""
    fw_types = (
        firmware_pb2.FirmwareArtifactInfo.TarballInfo.FirmwareType  # pylint: disable=no-member
    )
    info = firmware_pb2.FirmwareArtifactInfo()  # pylint: disable=no-member
    info.bcs_version_info.version_string = opts.bcs_version
    version = opts.bcs_version or get_version()
    bundle_dir = get_bundle_dir(opts)
    firmware_targets = opts.firmware_targets.split(",")
    for build_target in firmware_targets:
        tarball_dir = os.path.join(bundle_dir, build_target)
        if not os.path.isdir(tarball_dir):
            os.mkdir(tarball_dir)
        tarball_name = "firmware_from_source.tar.bz2"
        tarball_path = os.path.join(tarball_dir, tarball_name)
        artifacts_dir = pathlib.Path("/build") / build_target / "firmware"

        # Only package firmware if it exists and has files
        if artifacts_dir.is_dir():
            files_to_pack = [
                x.relative_to(artifacts_dir)
                for x in artifacts_dir.glob("*")
                if not x.name
                in (
                    "private-directories",
                    "coreboot-private",
                    "cbfs-ro-compress",
                    "cbfs-rw-compress-override",
                )
            ]
            if files_to_pack:
                cmd = [
                    "tar",
                    "cvfj",
                    tarball_path,
                    "--exclude=*.d",
                    "--exclude=*.o",
                    "--exclude=libpayload*",
                ]
                cmd.extend(files_to_pack)
                log_cmd(cmd, cwd=artifacts_dir)
                subprocess.run(
                    cmd,
                    check=True,
                    cwd=artifacts_dir,
                )

                meta = info.objects.add()
                meta.file_name = f"{build_target}/{tarball_name}"
                meta.tarball_info.type = fw_types.MAIN
                meta.tarball_info.firmware_image_name = build_target
                meta.tarball_info.board.append(build_target)

        # Collect and bundle ebuild logs using chromite helper
        try:
            chroot = chroot_lib.Chroot()
            sysroot = sysroot_lib.Sysroot(f"/build/{build_target}")

            logs_tarball_name = artifacts.BundleEBuildLogsTarball(
                chroot, sysroot, tarball_dir
            )

            if logs_tarball_name:
                # Register the logs tarball in metadata
                meta = info.objects.add()
                meta.file_name = f"{build_target}/{logs_tarball_name}"
                meta.tarball_info.type = fw_types.MAIN
                meta.tarball_info.board.append(build_target)
        except Exception as e:
            # Don't let log collection failure crash the bundler
            print(f"Failed to collect ebuild logs using helper: {e}")

        pattern = os.path.join(artifacts_dir, "image-*.bin")
        matched_files = glob.glob(pattern)
        for filepath in matched_files:
            filename = os.path.basename(filepath)
            if filename.count(".") != 1:
                continue
            match = re.search(r"^image-(.*)\.bin$", filename)
            if not match:
                continue
            model = match.group(1)
            # karis.15709.192.0.tar.bz2
            if version:
                tarball_name = f"{model}.{version}.tar.bz2"
            else:
                tarball_name = f"{model}.tar.bz2"
            # Package the bin file
            tarball_path = bundle_dir.joinpath(tarball_name)
            cmd = [
                "tar",
                "cvfj",
                tarball_path,
                filename,
            ]
            subprocess.run(
                cmd,
                check=True,
                cwd=artifacts_dir,
            )
            meta = info.objects.add()
            meta.file_name = tarball_name
            meta.tarball_info.type = fw_types.MAIN
            meta.tarball_info.board.append(build_target)
    write_metadata(opts, info)
    return 0


def parse_args(args):
    """Parse all command line args and return opts dict."""
    parser = argparse.ArgumentParser(description=__doc__)

    parser.add_argument(
        "--cpus",
        default=multiprocessing.cpu_count(),
        help="The number of cores to use.",
    )

    parser.add_argument(
        "--firmware-targets",
        required=True,
        help="Comma-separated list of firmware targets (~boards).",
    )

    parser.add_argument(
        "--metrics",
        dest="metrics",
        required=True,
        help="File to write the json-encoded MetricsList proto message.",
    )

    parser.add_argument(
        "--metadata",
        required=False,
        help="Full file pathname to write build artifact metadata in.",
    )

    parser.add_argument(
        "--output-dir",
        required=False,
        help="Full directory pathname to bundle build artifacts in.",
    )

    parser.add_argument(
        "--bcs-version",
        dest="bcs_version",
        default="",
        required=False,
        help="BCS version to include in metadata.",
    )

    parser.add_argument(
        "--avb-enabled",
        dest="avb_enabled",
        required=False,
        action="store_true",
        help='Build with the USE="avb".',
    )

    sub_cmds = parser.add_subparsers(required=True)

    build_cmd = sub_cmds.add_parser("build", help="Builds all firmware targets")
    build_cmd.set_defaults(func=build)

    test_cmd = sub_cmds.add_parser("test", help="Runs all firmware unit tests")
    test_cmd.set_defaults(func=test)

    test_cmd = sub_cmds.add_parser("bundle", help="Create an artifact tarball.")
    test_cmd.set_defaults(func=bundle)

    return parser.parse_args(args)


def main(args):
    opts = parse_args(args)

    if not hasattr(opts, "func"):
        print("Must select a valid sub command!")
        return -1

    # Run selected sub command function
    return opts.func(opts)


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
