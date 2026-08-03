#!/usr/bin/env python3
# Copyright 2021 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Get a copy of CrOS's kernel for use in borealis.

Builds the kernel used but borealis. This is done in the CrOS chroot using the
borealis board, in a similar fashion to how termina_tools.py works.
"""

import argparse
import logging
import os
import subprocess

import build_full
import chroot


KERNEL_FILENAME = "vm_kernel"
KERNEL_BOARD = "borealis"

# Any changes here should also be reflected in:
# src/private-overlays/overlay-borealis-private/profile/base/make.defaults
KERNEL_VERSION = "6_1"
KERNEL_OLD_VERSION = "5_15"


logger = logging.getLogger("borealis_kernel")


def make_package_name(version):
    """Determine the package name for a given kernel version."""
    return f"sys-kernel/chromeos-kernel-{version}"


def get_kernel(
    cros_path,
    chroot_path,
    out_dir,
    board,
    version,
    old_version,
    clean_build=False,
):
    """Builds a kernel in a CrOS chroot and copies it to the expected place."""
    logger.info("Building kernel")
    ch = chroot.Chroot(cros_path, board, chroot_path, out_dir)

    # TODO(davidriley): During the transition between kernel versions, try and
    # recover from a failed build by removing the old.
    package_name = make_package_name(version)
    try:
        ch.build_packages(package_name, clean_build)
    except subprocess.CalledProcessError:
        ch.unmerge_package(make_package_name(old_version))
        ch.build_packages(package_name, clean_build)
    # If the package is being cros_workon'd explicitly emerge it.
    # The build_packages is still done to do any dependency building in parallel.
    if ch.is_workon(package_name):
        ch.emerge_package(package_name)
    kernel = ch.get_build_path("boot", "vmlinuz")
    if not os.path.exists(kernel):
        raise Exception(f"Failed to build kernel; {kernel} does not exist")
    subprocess.check_call(["cp", kernel, KERNEL_FILENAME])


def main():
    """Command-line front-end used to get a kernel for borealis."""
    logging.basicConfig(
        level=logging.INFO,
        format="%(asctime)s: %(levelname)s: %(name)s: %(message)s",
        datefmt="%Y-%m-%d %H:%M:%S",
    )

    parser = argparse.ArgumentParser(description=__doc__)

    parser.add_argument(
        "--cros",
        default=build_full.get_cros_dir(),
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
        "--board",
        default=KERNEL_BOARD,
        help="CrOS board to use for building the kernel",
    )
    parser.add_argument(
        "--version",
        default=KERNEL_VERSION,
        help="Version of the kernel to build",
    )
    parser.add_argument(
        "--old_version",
        default=KERNEL_OLD_VERSION,
        help="Old version of kernel to clean if there is a build failure",
    )
    parser.add_argument(
        "--clean_build",
        action="store_true",
        help="Performs a clean build; Deletes sysroot before building.",
    )

    args = parser.parse_args()
    get_kernel(
        args.cros,
        args.chroot,
        args.out_dir,
        args.board,
        args.version,
        args.old_version,
        args.clean_build,
    )


if __name__ == "__main__":
    main()
