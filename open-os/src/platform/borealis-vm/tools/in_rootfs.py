#!/usr/bin/env python3
# Copyright 2021 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Runs a command inside a rootfs

Temporarily mounts the given rootfs image and runs the specified command in it.
"""

import argparse
import os
import subprocess
import tempfile

import sysroot_to_image


def run_cmd_in_rootfs(image_path, command):
    """Runs the supplied command inside a mounted rootfs (where cwd is '/').

    Args:
        image_path: path to the vm image you want to run a command in.
        command: a list of command arguments you want to run.
    """
    with tempfile.TemporaryDirectory() as mnt_dir:
        with sysroot_to_image.ScopedMount("ext4", image_path, mnt_dir):
            subprocess.check_call(
                ["sudo", "chroot", mnt_dir] + command, cwd=mnt_dir
            )


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("image", help="Path to the rootfs image")
    parser.add_argument(
        "cmd", help="Command you want to run in the rootfs", nargs="+"
    )
    args = parser.parse_args()
    run_cmd_in_rootfs(args.image, args.cmd)


if __name__ == "__main__":
    main()
