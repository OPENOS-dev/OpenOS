#!/usr/bin/env python3
# Copyright 2020 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Build and deploy the container tools to a borealis image.

Utility to help redeploy the container tools to a user-built borealis image
without having to re-build/deploy the whole image.

The container tools are all the binaries/libs built by CrOS's
'termina_container_tools' package, e.g. maitred, garcon, vsh. It does NOT
include sommelier, which is built differently.

You probably want to have cros_workon-ed 'vm_guest_tools' (and possibly
'vm_protos').
"""

import argparse
import contextlib
from os import path
import subprocess

import build_full


REMOTE_DIR = "/mnt/stateful_partition/img_borealis"
REMOTE_MNT = path.join(REMOTE_DIR, "TOOLS_TMP")
TOOLS_PATH = "opt/google/cros-containers"
PACKAGES = ["vm_protos", "vm_guest_tools", "termina_container_tools"]


def command(cmd, can_fail=False, working_dir="."):
    """Run a command-line command, with some helpful printouts.

    Args:
        cmd: command line to run.
        can_fail: if true, the command can fail (exit with nonzero).
        working_dir: cwd for the command.
    """
    print()
    print(" ".join(cmd))
    if can_fail:
        subprocess.call(cmd, cwd=working_dir)
    else:
        subprocess.check_call(cmd, cwd=working_dir)


def build(cros_path):
    """Build the termina tools as a tarball.

    Args:
        cros_path: path to the CrOS checkout.
    """
    command(
        ["cros_sdk", "emerge-borealis"] + PACKAGES,
        can_fail=False,
        working_dir=cros_path,
    )


def remote(dut_addr, cmd, can_fail=False):
    """Run a command remotely on a dut.

    Args:
        dut_addr: ssh hostname/IP for the dut you're running on.
        cmd: command line to run.
        can_fail: if true, the command can fail (exit with nonzero).
    """
    command(["ssh", dut_addr, "--"] + cmd, can_fail=can_fail)


@contextlib.contextmanager
def make_and_remove_tmp_dir(dut_addr):
    """Create a context where a directory exists to mount the rootfs image on.

    Args:
        dut_addr: ssh hostname/IP for the dut you're running on.
    """
    try:
        remote(dut_addr, ["mkdir", "-p", REMOTE_MNT])
        yield
    finally:
        remote(dut_addr, ["rmdir", REMOTE_MNT])


@contextlib.contextmanager
def mount_and_unmount_image(dut_addr):
    """Create a context where the remote image is mounted.

    Args:
        dut_addr: ssh hostname/IP for the dut you're running on.
    """
    remote_img = path.join(REMOTE_DIR, "vm_rootfs.img")
    try:
        remote(dut_addr, ["mount", remote_img, REMOTE_MNT])
        yield
    finally:
        remote(dut_addr, ["umount", REMOTE_MNT])


def deploy(cros_path, dut_addr, deploy_libs):
    """Put the termina tools onto a deployed borealis image.

    Args:
        cros_path: path to the CrOS checkout.
        dut_addr: ssh hostname/IP for the dut you're deploying to.
        deploy_libs: if true, deploys the libs/ subdir (in addition to bin/).
    """
    with make_and_remove_tmp_dir(dut_addr):
        remote(dut_addr, ["restart", "vm_concierge"], can_fail=True)
        remote(dut_addr, ["restart", "vm_cicerone"], can_fail=True)
        with mount_and_unmount_image(dut_addr):
            build_dir = path.join(cros_path, "chroot", "build", "borealis")
            deployed_dirs = [path.join(build_dir, TOOLS_PATH, "bin")]
            if deploy_libs:
                deployed_dirs.append(path.join(build_dir, TOOLS_PATH, "lib"))
            command(
                ["scp", "-r"]
                + deployed_dirs
                + [dut_addr + ":" + path.join(REMOTE_MNT, TOOLS_PATH)]
            )


def main():
    """Command-line front end for deploying container tools"""
    parser = argparse.ArgumentParser(description=__doc__)

    parser.add_argument(
        "--cros",
        default=build_full.get_cros_dir(),
        help="Path to the CrOS checkout (i.e. with src/ and chroot/ subdirs)",
    )
    parser.add_argument(
        "--dut",
        default="dut",
        help="IP address/hostname of the device you want to deploy to",
    )
    parser.add_argument(
        "--nobuild",
        action="store_false",
        dest="build",
        help="If set, skips (re)building the tools",
    )
    parser.add_argument(
        "--nodeploy",
        action="store_false",
        dest="deploy",
        help="If set, skips deploying the tools",
    )
    parser.add_argument(
        "--nolibs",
        action="store_false",
        dest="libs",
        help="If set, skip deploying the libs/ subdirectory (bin/ will still be deployed)",
    )

    args = parser.parse_args()
    if args.build:
        build(args.cros)
    if args.deploy:
        deploy(args.cros, args.dut, args.libs)


if __name__ == "__main__":
    main()
