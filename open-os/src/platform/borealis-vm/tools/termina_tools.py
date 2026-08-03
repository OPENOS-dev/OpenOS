#!/usr/bin/env python3
# Copyright 2020 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Build the termina tools for use in borealis.

Uses the CrOS build system to make the termina tools, and archives them for use
later. This process takes a long time so use only as needed. This has purposes:
1) To add local changes to CrOS tools to a locally built borealis image, 2) To
make archives for use in the automatic build.

This process is only necessary until we are able to build the termina tools
entirely in the borealis chroot.

To add new binaries here, see termina_container_tools-0.0.1.ebuild.
"""

import argparse
import datetime
import os

import chroot


DEFAULT_BOARD = "borealis"
# The list of files we know that we need.
EXPECTED_TOOLS = [
    "bin/core2md",
    "bin/crash_reporter",
    "bin/garcon",
    "bin/maitred",
    "bin/vshd",
    "etc/crash_reporter_logs.conf",
]


def default_output():
    """Return default output."""
    return (
        "termina_tools_"
        + datetime.datetime.now().strftime("%Y-%m-%d-%H%M%S")
        + ".tbz"
    )


def get_termina_tools(
    cros_path, output, board=DEFAULT_BOARD, chroot_path=None, out_dir=None
):
    """Create an archive of the termina tools.

    Invokes the CrOS build system to build the termina tools for your locally
    checked-out repo and archives them.

    Args:
        cros_path: path to the local CrOS checkout.
        output: path where the archive will be saved.
        board: name of the cros board which will be used as the target for the
          CrOS build. This should be an x86_64 board e.g. borealis, tatl.
        chroot_path: path to the CrOS chroot.
        out_dir: path to the CrOS output directory.

    Raises:
        Exception: if any of the expected tools are not built.
    """
    # Build the tools.
    ch = chroot.Chroot(cros_path, board, chroot_path, out_dir)
    ch.build_and_archive(
        "termina_container_tools",
        output,
        os.path.join("opt", "google", "cros-containers"),
        EXPECTED_TOOLS,
    )


def main():
    """Main function."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--cros",
        required=True,
        help="Path to the CrOS repo. This should have src/ and chroot/ subdirs",
    )
    parser.add_argument(
        "--output",
        default=default_output(),
        help="Location where the output is stored.",
    )
    parser.add_argument(
        "--board",
        default=DEFAULT_BOARD,
        help="Board to use when building the tools.",
    )

    args = parser.parse_args()
    get_termina_tools(args.cros, args.output, args.board)


if __name__ == "__main__":
    main()  # pylint: disable=no-value-for-parameter
