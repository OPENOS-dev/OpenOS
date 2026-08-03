#!/usr/bin/env python3
# Copyright 2024 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Build XWayland in Docker and extract it out to host directory."""

# TODO(endlesspring): refactor deployment functions at deploy_container_tools.py then make this
# script into deploy_xwayland.py.


import argparse
import logging
import os
import pathlib
import subprocess
import sys

import build_full


class _MainArgs:
    xwayland_source: pathlib.Path
    output: pathlib.Path


def main(args: _MainArgs) -> int:
    """Command-line tool for building XWayland"""
    # Prepare output path
    if not os.path.exists(args.output):
        os.makedirs(args.output)
    xwayland_output_path = os.path.join(args.output, "Xwayland")
    if not os.path.isdir(args.output):
        logging.error("%s is not a directory", args.output)
        return 1
    if os.path.exists(xwayland_output_path):
        # Explicitly remove the file so that if the build fails, the user does not assume the
        # existing file as successful output of the build.
        subprocess.check_call(["sudo", "rm", xwayland_output_path])
        logging.info("Removed existing %s", xwayland_output_path)

    # Get the directory that has PKGBUILD and build_local.sh
    xwayland_package_root = os.path.join(
        build_full.get_build_dir(),
        "packages",
        "cros-xwayland",
    )
    if not os.path.exists(xwayland_package_root):
        logging.error("%s does not exist", xwayland_package_root)
        return 1

    logging.info("Adding o+rx to %s", args.xwayland_source)
    subprocess.check_call(["chmod", "-R", "o+rx", args.xwayland_source])

    logging.info("Building cros_xwayland image")
    build_full.build_full(
        build_dir=build_full.get_build_dir(),
        image_name="cros_xwayland",
        cros_path=build_full.get_cros_dir(),
        stage="xwayland-builder",
        no_cache=False,
        skip_termina=True,
        run_unit_tests=False,
    )

    logging.info("Building Xwayland")
    subprocess.check_call(
        [
            "sudo",
            "docker",
            "run",
            "-i",
            "-v",
            f"{args.xwayland_source}:/scratch/xwayland",
            "-v",
            f"{xwayland_package_root}:/scratch/pkgbuild",
            "-v",
            f"{args.output}:/scratch/output",
            "-t",
            "cros_xwayland:latest",
            "bash",
            "/scratch/pkgbuild/build_local.sh",
            "/scratch/xwayland",
            "/scratch/output",
        ],
    )
    logging.info(
        "Successfully built and extracted Xwayland binary:\n%s\n"
        "Use copy&replace this file into your borealis image's /usr/bin/Xwayland\n\n"
        "Example (temporarily replace for this borealis run):\n"
        "$ ssh $DUT -- borealis-sh --user=root -- mount -o rw,remount /\n"
        "$ cat %s | ssh $DUT -- borealis-sh --user=root dd of=/home/chronos/Xwayland\n"
        "$ ssh $DUT -- borealis-sh --user=root chmod 755 /home/chronos/Xwayland\n"
        "$ ssh $DUT -- borealis-sh --user=root mv /home/chronos/Xwayland"
        " /usr/bin/Xwayland\n"
        "$ ssh $DUT -- borealis-sh --user=root -- pkill sommelier\n",
        xwayland_output_path,
        xwayland_output_path,
    )
    return 0


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--output",
        default="/tmp/borealis-xwayland",
        help="Path to export Xwayland binary to.",
        type=pathlib.Path,
    )
    parser.add_argument(
        "--xwayland-source",
        required=True,
        help="Path to xserver source.",
        type=pathlib.Path,
    )
    logging.basicConfig(
        level=logging.INFO,
        format="%(asctime)s: %(levelname)s: %(name)s: %(message)s",
        datefmt="%Y-%m-%d %H:%M:%S",
    )
    sys.exit(main(parser.parse_args(namespace=_MainArgs)))
