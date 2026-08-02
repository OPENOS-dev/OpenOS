#!/usr/bin/env python3
# Copyright 2021 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Build packages for a vendor-specific image.

Uses the CrOS build system to build a package, and archives up files to
use later.
"""

import argparse
import datetime

import build_full
import chroot


DEFAULT_BOARD = "borealis"


def default_output():
    """Generate a default output tarball filename."""
    return (
        "vendor_tools_"
        + datetime.datetime.now().strftime("%Y-%m-%d-%H%M%S")
        + ".tbz"
    )


def main():
    """Generate a vendor tarball."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--board",
        default=DEFAULT_BOARD,
        help="Board to use when building the tools.",
    )
    parser.add_argument(
        "--cros",
        default=build_full.get_cros_dir(),
        help="Path to the CrOS repo. This should have src/ and chroot/ subdirs",
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
        "--output",
        default=default_output(),
        help="Location where the output tarball is written to.",
    )
    parser.add_argument(
        "--package", required=True, help="Package name to emerge."
    )
    parser.add_argument(
        "--paths", nargs="*", help="Paths to tar (relative to root arg)."
    )
    parser.add_argument(
        "--root",
        default=".",
        help="Root directory to tar from (relative to board build dir).",
    )

    args = parser.parse_args()

    # Build and archive the tools.
    ch = chroot.Chroot(
        args.cros,
        board=args.board,
        chroot_path=args.chroot,
        out_dir=args.out_dir,
    )
    ch.build_and_archive(
        args.package, args.output, args.root, archive_paths=args.paths
    )


if __name__ == "__main__":
    main()  # pylint: disable=no-value-for-parameter
