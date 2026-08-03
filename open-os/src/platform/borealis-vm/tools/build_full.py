#!/usr/bin/env python3
# Copyright 2020 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Build the borealis image.

Uses docker and some of the other tools to construct the borealis image.
"""

import argparse
import contextlib
import logging
import os
import re
import subprocess
import sys
import tarfile
import tempfile

import chroot
import generate_license_html
import generate_metadata_xml
import prepare_vendor
import run_tests
import sysroot_to_image
import termina_tools

import config


logger = logging.getLogger("build_full")


def get_borealis_dir():
    """Path to the main borealis-vm/ directory"""
    # This script is in borealis-vm/tools/__file__ so we want 2 levels up.
    return os.path.dirname(os.path.dirname(os.path.realpath(__file__)))


def get_borealis_private_dir():
    """Path to the main borealis-private/ directory"""
    # This script is in borealis/tools/__file__ so we want 3 levels up and
    # append 'borealis-private'
    return os.path.join(
        os.path.dirname(
            os.path.dirname(os.path.dirname(os.path.realpath(__file__)))
        ),
        "borealis-private/",
    )


# TODO(hollingum): Currently many of the docker dependencies are subdirectories
# of build/, meaning that we must make it the working directory for the docker
# build. Ideally we would separate these dependencies into versioned repos that
# are checked out into an arbitrary working dir.
def get_build_dir():
    """Path to the build/ directory"""
    return os.path.join(get_borealis_dir(), "build")


def get_cros_dir():
    """Path to the CrOS repo containing this borealis subrepo"""
    # The borealis repo lives in src/platform/borealis, so we want 3 levels
    # above borealis itself.
    return os.path.dirname(os.path.dirname(os.path.dirname(get_borealis_dir())))


def get_dlc_metadata_path(dlc="borealis-dlc"):
    """Path to the borealis-dlc metadata.xml file."""
    return os.path.join(
        get_cros_dir(),
        "src/private-overlays/chromeos-partner-overlay/chromeos-base",
        dlc,
        "metadata.xml",
    )


def get_subrepo_version(cros_path, subrepo):
    """Returns the version of the given subrepo in r{ORDER}.{HASH} format.

    Args:
        cros_path: path to the CrOS checkout.
        subrepo: path under src/ that you want the stamp for
    """
    subrepo_dir = os.path.join(cros_path, "src", *subrepo)
    ord_out = subprocess.check_output(
        ["git", "rev-list", "--count", "HEAD"], cwd=subrepo_dir
    )
    hash_out = subprocess.check_output(
        ["git", "rev-parse", "--short", "HEAD"], cwd=subrepo_dir
    )
    return "r%s.%s" % (  # pylint: disable=consider-using-f-string
        ord_out.decode(sys.stdout.encoding).strip(),
        hash_out.decode(sys.stdout.encoding).strip(),
    )


def stamp(build_dir, name, value):
    """Adds stamp info to the build.

    Args:
        build_dir: path to the directory where the build will happen (i.e. where
          the archive will be stored).
        name: name of this stamp, will form part of the lsb-release key.
        value: the stamp's contents.
    """
    os.makedirs(os.path.join(build_dir, "stamp"), exist_ok=True)
    stamp_file = os.path.join(build_dir, "stamp", name)
    with open(stamp_file, "w", encoding="utf-8") as f:
        f.write(value)
    return stamp_file


def get_termina_tools(build_dir, cros_path, chroot_path=None, out_dir=None):
    """Compiles the termina tools archive.

    Args:
        build_dir: path to the directory where the build will happen (i.e. where
          the archive will be stored).
        cros_path: path to the CrOS checkout, which we use to build the tools.
        chroot_path: path to the CrOS chroot (usually $cros_path/chroot).
        out_dir: path to the CrOS out dir (usually $cros_path/out).
    """
    # TODO(hollingum): Support the use-case of downloading the tools archive
    # from a gsutil bucket.
    tools_file = os.path.join(build_dir, "termina_tools.tbz")
    if cros_path is None:
        raise ValueError("Unable to build termina tools, '--cros' not supplied")
    termina_tools.get_termina_tools(
        cros_path, output=tools_file, chroot_path=chroot_path, out_dir=out_dir
    )
    stamp(build_dir, "platform2", get_subrepo_version(cros_path, ["platform2"]))


@contextlib.contextmanager
def setup_code(build_dir, cros_path):
    """Puts CrOS code in a place accessible to the build.

    Some of the build steps require code stored in the CrOS repo. We mount these
    into a known subdir of the build directory so they can be used by docker.
    """
    cros_sommelier = os.path.join(
        cros_path, "src", "platform2", "vm_tools", "sommelier"
    )
    build_sommelier = os.path.join(build_dir, "sommelier")
    # We don't clean up this directory, the presence of the empty directory is
    # a hint to the developer, but git doesn't like empty dirs.
    os.makedirs(build_sommelier, exist_ok=True)
    borealis_stamp = stamp(
        build_dir,
        "borealis",
        get_subrepo_version(cros_path, ["platform", "borealis-vm"]),
    )
    sommelier_stamp = stamp(
        build_dir, "sommelier", get_subrepo_version(cros_path, ["platform2"])
    )
    with sysroot_to_image.ScopedMount("bind", cros_sommelier, build_sommelier):
        yield None
    os.remove(sommelier_stamp)
    os.remove(borealis_stamp)


def build_via_docker(
    build_dir, image_name, stage, no_cache=False, build_arg=None
):
    """Build the borealis rootfs as a docker image"""
    args = [
        "sudo",
        "env",
        "DOCKER_BUILDKIT=1",
        "docker",
        "build",
        "-t",
        image_name,
        build_dir,
        "--target",
        stage,
    ]
    if no_cache:
        args.append("--no-cache")
    if build_arg:
        for arg in build_arg:
            args.extend(["--build-arg", arg])
    logger.info("Running: %s", " ".join(args))
    subprocess.check_call(args)


def dump_package_list(image_name, package_list):
    """Dump the list of packages according to pacman."""
    cmd = [
        "sudo",
        "docker",
        "run",
        "--rm",
        image_name,
        "pacman",
        "-Qi",
    ]
    logger.info("Exporting package list with pacman -Qi")
    packages = subprocess.check_output(cmd).decode(sys.stdout.encoding)
    with open(package_list, "w", encoding="utf-8") as f:
        f.write(packages)


def dump_licenses_tar(image_name, licenses_tar):
    """Create a tarball of the licenses in an image."""

    # /usr/share/licenses might contain absolute links to /usr/share/docs
    # so this is done from within the container.  The resulting licenses.tar
    # has a directory licenses filled with files and no symlinks.
    cmd = [
        "sudo",
        "docker",
        "run",
        "--rm",
        image_name,
        "tar",
        "-C",
        os.path.join("/usr", "share"),
        # Include symlink contents.
        "--dereference",
        "-cf",
        "-",
        "licenses",
    ]
    logger.info("Extracting /usr/share/licenses from image")
    tar_data = subprocess.check_output(cmd)
    with open(licenses_tar, "wb") as f:
        f.write(tar_data)


def dump_packages_tar(image_name, packages_tar):
    """Create a tarball of the packages in an image."""

    cmd = [
        "sudo",
        "docker",
        "run",
        "--rm",
        image_name,
        "tar",
        "-C",
        os.path.join("/scratch", "pkgs"),
        "-cf",
        "-",
        ".",
    ]
    logger.info("Extracting /scratch/pkgs from image")
    tar_data = subprocess.check_output(cmd)
    with open(packages_tar, "wb") as f:
        f.write(tar_data)


def generate_breakpad(ch, breakpad_image_name, breakpad_tar):
    """Generate a tarball of breakpad symbols to upload."""
    # Work will all happen in a temporary directory that is accessible
    # from inside and outside the chroot context:
    # ${cros_path}/build/borealis/tmp/tmpXXXXXX
    ch_tmp = ch.get_build_path("tmp")
    with tempfile.TemporaryDirectory(dir=ch_tmp) as tempdir:
        # Get all the packages from the Docker build.
        packages_tar = os.path.join(tempdir, "packages.tar")
        dump_packages_tar(breakpad_image_name, packages_tar)
        packages = []

        # Untar all the debug packages, remembering their names.
        logger.info("Untarring packages")
        with tarfile.open(packages_tar) as tf:
            members = [
                m for m in tf.getmembers() if re.search(r"-debug-", m.name)
            ]
            packages = [m.name for m in members]
            tf.extractall(tempdir, members=members)

        # Generate breakpad symbols for each package into
        # ${cros_path}/build/borealis/tmp/tmpXXXXXX/breakpad
        # Need an absolute path that works within the chroot for the tempdir.
        ch_tempdir = ch.as_chroot_path(tempdir, absolute=True)
        for p in packages:
            logger.info("Generating breakpad symbols for package %s", p)
            ch.generate_breakpad_symbols(
                os.path.join(ch_tempdir, p),
                os.path.join(ch_tempdir, "breakpad"),
            )

        # TODO(davidriley): This should also capture debug symbols
        # for the kernel and borealis cros side build.

        # create_archive() normally expects a relative path to
        # get_build_path() but that's a bit annoying to calculate since
        # we've got an absolute path from TemporaryDirectory so just
        # regenerate an absolute path for create archive.
        logger.info("Creating breakpad tarball %s", breakpad_tar)
        breakpad_dir = os.path.join(tempdir, "breakpad")
        ch.create_archive(breakpad_tar, breakpad_dir)


def generate_licenses(
    image_name, license_html, package_list=None, licenses_tar=None
):
    """Generate a license summary."""
    with tempfile.TemporaryDirectory() as tempdir:
        # Generate package list and licenses tarball if not provided.
        if not package_list:
            package_list = os.path.join(tempdir, "packages.txt")
            dump_package_list(image_name, package_list)
        if not licenses_tar:
            licenses_tar = os.path.join(tempdir, "licenses.tar")
            dump_licenses_tar(image_name, licenses_tar)

        # Untar licenses.
        cmd = [
            "tar",
            "-C",
            tempdir,
            "-xf",
            licenses_tar,
        ]
        subprocess.check_call(cmd)

        # TODO(b/353582114) Remove glu/license.txt when shipped with Arch.
        # TODO(b/353583792) Remove *libgpg-error/COPYING.other when shipped with Arch.
        # TODO(b/313945837) Remove Linux-syscall-note.txt when shipped with Arch.
        # TODO(b/328466486) Remove systemd/MIT-0.txt when shipped with Arch.
        # TODO(b/330368367) Remove python-dnslib/LICENSE when shipped with Arch.
        logger.info("Adding extra licenses missing from Arch")
        cmd = [
            "cp",
            "-avr",
            os.path.join(get_borealis_dir(), "tools", "licenses"),
            tempdir,
        ]
        subprocess.check_call(cmd)

        html = generate_license_html.prepare_html(
            package_list, os.path.join(tempdir, "licenses")
        )
        with open(license_html, "w", encoding="utf-8") as f:
            f.write(html)
        logger.info("Licenses written to %s", license_html)


def generate_metadata(
    image_name,
    metadata_xml_path,
    missing_mappings_path=None,
    exclude_existing_metadata_paths=None,
):
    """Generate CPE metadata.xml file."""

    xml_content, missing_mappings = generate_metadata_xml.generate_metadata_xml(
        image_name,
        exclude_existing_metadata_paths=exclude_existing_metadata_paths,
    )
    with open(metadata_xml_path, "wb") as f:
        f.write(xml_content)
    logger.info("CPE metadata written to %s", metadata_xml_path)

    # Show reverse dependencies of any packages that are missing mappings,
    # so we can figure out why they're installed.
    if missing_mappings:
        logger.info("Some CPE mappings are missing. Installing pactree...")
        subprocess.run(
            [
                "sudo",
                "docker",
                "run",
                image_name,
                "bash",
                "-c",
                "pacman -Sy --noconfirm pacman-contrib && "
                + r"""echo -e "\n====== Reverse deps of packages missing CPE mappings:" && """
                + "; ".join([f'pactree -r "{p}"' for p in missing_mappings]),
            ],
            check=False,  # This is informational only.
        )

    if missing_mappings_path:
        with open(missing_mappings_path, "w", encoding="utf-8") as f:
            for package in missing_mappings:
                f.write(f"{package}\n")
        if missing_mappings:
            logger.info(
                "Missing mappings list written to %s", missing_mappings_path
            )
            raise AttributeError(
                "Not all packages have a CPE mapping in tools/cpe/cpe_mappings.json."
                "  See go/borealis-vuln-scanning to fix."
            )
        logger.info(
            "CPE mapping file is complete; leaving %s empty",
            missing_mappings_path,
        )


def build_full(
    *,
    build_dir,
    image_name,
    cros_path,
    stage,
    chroot_path=None,
    out_dir=None,
    no_cache=False,
    build_arg=None,
    skip_termina=False,
    package_list=None,
    licenses_tar=None,
    licenses_html=None,
    metadata_xml=None,
    exclude_metadata=None,
    missing_mappings=None,
    breakpad_stage=None,
    breakpad_tar=None,
    run_unit_tests=False,
):
    """Build the borealis image and all associated tools (if needed)"""
    ch = chroot.Chroot(
        cros_path, termina_tools.DEFAULT_BOARD, chroot_path, out_dir
    )
    if not skip_termina:
        get_termina_tools(build_dir, cros_path, chroot_path, out_dir)
    with setup_code(build_dir, cros_path):
        build_via_docker(build_dir, image_name, stage, no_cache, build_arg)
    if package_list:
        dump_package_list(image_name, package_list)
    if licenses_tar:
        dump_licenses_tar(image_name, licenses_tar)
    if licenses_html:
        generate_licenses(image_name, licenses_html, package_list, licenses_tar)
    if metadata_xml:
        generate_metadata(
            image_name,
            metadata_xml,
            missing_mappings,
            exclude_existing_metadata_paths=exclude_metadata,
        )
    if breakpad_stage and breakpad_tar:
        breakpad_image_name = image_name + "_breakpad"
        with setup_code(build_dir, cros_path):
            build_via_docker(build_dir, breakpad_image_name, breakpad_stage)
        generate_breakpad(ch, breakpad_image_name, breakpad_tar)
    if run_unit_tests:
        run_tests.run_rootfs_tests(image_name)


def verify_arch_databases():
    """Call the Arch checksum tool to verify the database files."""
    verity_cmd = [
        f"{get_borealis_dir()}/tools/arch_verity/checksum_tool.py",
        "--verify",
    ]
    subprocess.check_call(verity_cmd)


def main():
    """Command-line front end for building a borealis image"""
    logging.basicConfig(
        level=logging.INFO,
        format="%(asctime)s: %(levelname)s: %(name)s: %(message)s",
        datefmt="%Y-%m-%d %H:%M:%S",
    )

    parser = argparse.ArgumentParser(description=__doc__)

    parser.add_argument(
        "--cros",
        default=get_cros_dir(),
        help="Path to the CrOS checkout (i.e. with src/ and chroot/ subdirs)",
    )
    parser.add_argument(
        "--out-dir", default=None, help="Path to the CrOS output directory."
    )
    parser.add_argument(
        "--chroot",
        default=None,
        help="Path to the CrOS chroot (i.e. with build/ subdir)",
    )
    parser.add_argument(
        "--build-dir",
        default=get_build_dir(),
        help="Path to the Docker build directory (i.e. with Dockerfile)",
    )
    parser.add_argument(
        "--no-cache",
        action="store_true",
        help="Do not use cached image artifacts and build from scratch.",
    )
    parser.add_argument(
        "--skip-termina",
        action="store_true",
        help="Do not try to rebuild the termina tools archive.",
    )
    parser.add_argument(
        "--stage", default="runtime_prod", help="Stage to build."
    )
    parser.add_argument(
        "--image-name",
        default=config.BOREALIS_TAG,
        help="Name (docker tag) for the image built by docker.",
    )
    parser.add_argument(
        "--build-arg",
        default=[],
        action="append",
        help="Build args to pass to docker build",
    )
    parser.add_argument(
        "--package-list", help="Filename to write list of packages to."
    )
    parser.add_argument(
        "--licenses-tar", help="Filename to write licenses tarball to."
    )
    parser.add_argument(
        "--licenses-html",
        default="credits.html",
        help="Filename to write license summary to.",
    )
    parser.add_argument(
        "--metadata-xml",
        default="metadata.xml",
        help="Filename to write CPE metadata to.",
    )
    parser.add_argument(
        "--missing-mappings",
        default="missing_mappings.txt",
        help="Filename to write missing CPE mapping list to.",
    )
    parser.add_argument(
        "--skip-license-report",
        action="store_true",
        default=False,
        help="Skip writing license tarball and summary",
    )
    parser.add_argument(
        "--breakpad-stage",
        default="runtime_pkgs",
        help="Stage with packages in /scratch/pkgs.",
    )
    parser.add_argument(
        "--breakpad-tar", help="Filename to write breakpad tbz2 to."
    )
    # TODO(b/236270403) Once the chroot is on Python 3.9, replace these with a
    # single add_argument call with action=argparse.BooleanOptionalAction.
    parser.add_argument(
        "--run-tests",
        action="store_true",
        default=True,
        help="Run unit tests after build.",
    )
    parser.add_argument(
        "--no-run-tests",
        dest="run_tests",
        action="store_false",
        help="Skip unit tests after build.",
    )
    parser.add_argument("--variant", help="Variant image to built.")
    parser.add_argument(
        "--disable-arch-sig-validation",
        dest="disable_arch_sig_validation",
        action="store_true",
        help="Disable the checking of signatures on Arch packages.",
    )

    args = parser.parse_args()
    if args.skip_license_report:
        args.licenses_tar = None
        args.licenses_html = None

    if args.disable_arch_sig_validation:
        args.build_arg.append("disable_arch_sig_validation=true")

    if args.stage.endswith("-build") or args.stage.endswith("-builder"):
        # Don't run unit tests when building a builder or a single
        # package; they fail on anything above `runtime`.
        args.run_tests = False

    if not args.variant:
        build_full(
            build_dir=args.build_dir,
            image_name=args.image_name,
            cros_path=args.cros,
            stage=args.stage,
            chroot_path=args.chroot,
            out_dir=args.out_dir,
            no_cache=args.no_cache,
            build_arg=args.build_arg,
            skip_termina=args.skip_termina,
            package_list=args.package_list,
            licenses_tar=args.licenses_tar,
            licenses_html=args.licenses_html,
            metadata_xml=args.metadata_xml,
            missing_mappings=args.missing_mappings,
            breakpad_stage=args.breakpad_stage,
            breakpad_tar=args.breakpad_tar,
            run_unit_tests=args.run_tests,
        )
        # TODO(pobega): support this check for vendor builds as well
        if args.disable_arch_sig_validation:
            verify_arch_databases()
        return
    if args.variant == "chroot":
        build_chroot(args)
        return
    if args.variant == "nvidia":
        build_nvidia(args, parent_stage="runtime", target_stage="nvidia_prod")
        return
    if args.variant == "nvidia_test":
        build_nvidia(
            args, parent_stage="runtime_test", target_stage="nvidia_test"
        )
        return
    if args.variant == "nvidia_debug":
        build_nvidia(
            args, parent_stage="runtime_debug", target_stage="nvidia_test"
        )
        return
    logger.error("unknown variant %s", args.variant)


def build_chroot(
    args, parent_stage="runtime_debug", target_stage="runtime_chroot"
):
    """Build the borealis image with borealis_chroot dependencies"""
    # Build the runtime_debug stage.
    build_arg = [
        "BUILD_NATIVE_DRIVERS=1",  # Build native graphics drivers.
    ]
    build_arg += args.build_arg if args.build_arg else []
    build_full(
        build_dir=args.build_dir,
        image_name=f"{args.image_name}:{parent_stage}",
        cros_path=args.cros,
        stage=parent_stage,
        chroot_path=args.chroot,
        out_dir=args.out_dir,
        no_cache=args.no_cache,
        build_arg=build_arg,
        skip_termina=args.skip_termina,
        package_list=args.package_list,
        licenses_tar=args.licenses_tar,
        licenses_html=args.licenses_html,
        breakpad_stage=args.breakpad_stage,
        breakpad_tar=args.breakpad_tar,
        run_unit_tests=args.run_tests,
    )

    # Run Dockerfile located in vendor/chroot.
    build_arg = [
        f"PARENT_IMAGE={args.image_name}:{parent_stage}",
    ]
    build_arg += args.build_arg if args.build_arg else []
    build_full(
        build_dir="vendor/chroot",
        image_name=args.image_name,
        cros_path=args.cros,
        stage=target_stage,
        chroot_path=args.chroot,
        out_dir=args.out_dir,
        skip_termina=True,
        build_arg=build_arg,
        metadata_xml=args.metadata_xml,
        exclude_metadata=["*"],
        missing_mappings=args.missing_mappings,
    )
    logger.info("chroot build complete.")


def build_nvidia(
    args,
    *,
    parent_stage,
    target_stage,
    output="nvidia-modules.tbz",
    output_nv_tools="borealis-nvidia-tools.tbz",
):
    """Build the borealis image with nvidia specific libraries and associated tools"""
    # Build the parent stage.
    build_full(
        build_dir=args.build_dir,
        image_name=f"{args.image_name}:{parent_stage}",
        cros_path=args.cros,
        stage=parent_stage,
        chroot_path=args.chroot,
        out_dir=args.out_dir,
        no_cache=args.no_cache,
        build_arg=args.build_arg,
        skip_termina=args.skip_termina,
        package_list=args.package_list,
        licenses_tar=args.licenses_tar,
        licenses_html=args.licenses_html,
        breakpad_stage=args.breakpad_stage,
        breakpad_tar=args.breakpad_tar,
        run_unit_tests=args.run_tests,
    )

    # Build nvidia-drivers package tarball from gentoo.
    nvidia_folder = f"{get_borealis_private_dir()}/vendor/nvidia/"

    # Prepare clean vendor archive.
    ch = chroot.Chroot(
        args.cros, prepare_vendor.DEFAULT_BOARD, args.chroot, args.out_dir
    )
    ch.build_and_archive(
        "nvidia-drivers",
        f"{nvidia_folder}/{output}",
        ".",
        archive_paths=["lib/modules", "lib/firmware", "packages/x11-drivers"],
    )

    ch.build_and_archive(
        "borealis-nvidia-tools",
        f"{nvidia_folder}/{output_nv_tools}",
        ".",
        archive_paths=["opt/nvidia/cros-containers"],
    )

    # Run Dockerfile located in vendor/nvidia.
    build_arg = [
        f"PARENT_IMAGE={args.image_name}:{parent_stage}",
        f"NVIDIA_TBZ={output}",
        f"NVIDIA_TOOLS_TBZ={output_nv_tools}",
    ]
    build_arg += args.build_arg if args.build_arg else []
    build_full(
        build_dir=nvidia_folder,
        image_name=args.image_name,
        cros_path=args.cros,
        stage=target_stage,
        chroot_path=args.chroot,
        out_dir=args.out_dir,
        skip_termina=args.skip_termina,
        build_arg=build_arg,
        metadata_xml=args.metadata_xml,
        exclude_metadata=[get_dlc_metadata_path()],
        missing_mappings=args.missing_mappings,
    )
    logger.info("nvidia build complete.")


if __name__ == "__main__":
    main()
