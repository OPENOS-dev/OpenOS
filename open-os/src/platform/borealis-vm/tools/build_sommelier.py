#!/usr/bin/env python3
# Copyright 2024 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Build Sommelier in Docker and extract it out to host directory."""

# TODO(endlesspring): refactor deployment functions at deploy_container_tools.py then make this
# script into deploy_sommelier.py.

import argparse
import logging
import os
import pathlib
import subprocess
import sys

import build_full


class _MainArgs:
    output: pathlib.Path


def main(args: _MainArgs) -> int:
    """Command-line tool for building Sommelier"""
    sommelier_output_path = os.path.join(args.output, "sommelier")
    if not os.path.isdir(args.output):
        logging.error("%s is not a directory", args.output)
        return 1
    if os.path.exists(sommelier_output_path):
        # Explicitly remove the file so that if the build fails, the user does not assume the
        # existing file as successful output of the build.
        subprocess.check_call(["sudo", "rm", sommelier_output_path])
        logging.info("Removed existing %s", sommelier_output_path)

    logging.info("Building cros_sommelier image")
    build_full.build_full(
        build_dir=build_full.get_build_dir(),
        image_name="cros_sommelier",
        cros_path=build_full.get_cros_dir(),
        stage="sommelier-build",
        no_cache=False,
        skip_termina=True,
        run_unit_tests=False,
    )

    logging.info("Launching container with cros_sommelier image")
    # Alternatively, we can do `docker run` with tar to get the files inside the image,
    # https://stackoverflow.com/a/40608752
    container_id = (
        subprocess.check_output(
            ["sudo", "docker", "create", "cros_sommelier:latest"]
        )
        .decode("utf-8")
        .strip("\n")
    )
    logging.info("Created container %s", container_id)

    logging.info("Extracting out sommelier binary from the container")
    subprocess.check_call(
        [
            "sudo",
            "docker",
            "cp",
            f"{container_id}:/scratch/sommelier-git/src/sommelier/_build/sommelier",
            sommelier_output_path,
        ]
    )

    logging.info("Deleting the container")
    subprocess.check_call(["sudo", "docker", "rm", "-v", container_id])

    logging.info(
        "Successfully built and extracted sommelier binary:\n%s\n"
        "Use copy&replace this file into your borealis image's /usr/local/bin/sommelier\n\n"
        "Example (temporarily replace for this borealis run):\n"
        "$ ssh $DUT -- borealis-sh --user=root -- mount -o rw,remount /\n"
        "$ cat %s | ssh $DUT -- borealis-sh --user=root dd of=/home/chronos/sommelier\n"
        "$ ssh $DUT -- borealis-sh --user=root chmod 755 /home/chronos/sommelier\n"
        "$ ssh $DUT -- borealis-sh --user=root mv /home/chronos/sommelier"
        " /usr/local/bin/sommelier\n"
        "$ ssh $DUT -- borealis-sh --user=root -- pkill sommelier\n",
        sommelier_output_path,
        sommelier_output_path,
    )
    return 0


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--output",
        default="/tmp",
        help="Path to export sommelier binary to.",
        type=pathlib.Path,
    )
    logging.basicConfig(
        level=logging.INFO,
        format="%(asctime)s: %(levelname)s: %(name)s: %(message)s",
        datefmt="%Y-%m-%d %H:%M:%S",
    )
    sys.exit(main(parser.parse_args(namespace=_MainArgs)))
