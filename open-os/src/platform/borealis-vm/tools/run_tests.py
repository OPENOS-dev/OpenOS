#!/usr/bin/env python3
# Copyright 2022 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Run Borealis unit tests.

Uses docker and some of the other tools to construct the borealis image.
"""

import argparse
import os
import subprocess

import config


def borealis_dir():
    """Path to the borealis repo root directory."""
    return os.path.dirname(os.path.dirname(os.path.realpath(__file__)))


def run_local_tests():
    """Run local unit tests, i.e. ones that can run outside Docker."""
    print("Running local unit tests.")
    subprocess.check_call(
        [
            "/usr/bin/python3",
            "-m",
            "unittest",
            "discover",
            "-v",
            "--pattern",
            "*_test.py",
            "-s",
            os.path.join(borealis_dir(), "tools"),
        ]
    )


def run_kabuto_tests(verbose):
    """Run tests for tools/kabuto."""
    print("Running Kabuto tests.")
    subprocess.check_call(
        [os.path.join(borealis_dir(), "tools", "kabuto", "run_tests")]
        + (["--verbose"] if verbose else ["--no-verbose"])
    )


def run_rootfs_tests(image_name):
    """Run rootfs unit tests."""
    cmd = [
        "sudo",
        "docker",
        "run",
        "--rm",
        "-v",
        os.path.join(borealis_dir(), "test") + ":/test",
        image_name,
        "/test/run_tests",
    ]
    print(f"Running rootfs unit tests against Docker image {image_name}.")
    subprocess.check_call(cmd)


def run_window_trace_golden_tests():
    """Check the output of trace_window_system.py didn't change unexpectedly."""
    subprocess.check_call(
        [
            os.path.join(
                borealis_dir(), "tools", "windowtrace", "run_golden_tests.py"
            )
        ]
    )


def main():
    """Main entry point when run as tools/run_tests.py."""
    parser = argparse.ArgumentParser(description=__doc__)

    parser.add_argument(
        "files_modified",
        nargs="*",
        help="List of files modified in borealis repository",
    )
    parser.add_argument(
        "--verbose",
        default=True,
        action=argparse.BooleanOptionalAction,  # pylint: disable=no-member
        help="Output detailed test information and logs.",
    )
    parser.add_argument(
        "--docker",
        default=True,
        action=argparse.BooleanOptionalAction,  # pylint: disable=no-member
        help="Run Docker-based rootfs tests. Note disabling this does not disable windowtrace "
        "golden tests, even though they use Docker.",
    )
    parser.add_argument(
        "--image-name",
        default=config.BOREALIS_TAG,
        help="Name (Docker tag) for the image to test.",
    )

    args = parser.parse_args()

    # Detect local tests to run if a list of files has been provided,
    # which probably means we're being run as part of a presubmit.
    if args.files_modified:
        # A list of modified files has been provided, so we're probably running a presubmit.
        # Only run tests which are relevant to the modified files.
        test_local = test_kabuto = test_window_trace = False
        for file_name in args.files_modified:
            if (
                "tools/windowtrace" in file_name
                or file_name.endswith("trace_window_system.py")
                or "pacman.d.for-tools-image/" in file_name
            ):
                test_window_trace = True
            elif "tools/kabuto" in file_name:
                test_kabuto = True
            elif "tools/" in file_name:
                test_local = True
    else:
        # When invoked outside a presubmit, test everything.
        test_local = test_kabuto = test_window_trace = True

    if test_local:
        run_local_tests()
    if test_kabuto:
        run_kabuto_tests(verbose=args.verbose)
    if test_window_trace:
        run_window_trace_golden_tests()
    if args.docker:
        run_rootfs_tests(image_name=args.image_name)


if __name__ == "__main__":
    main()
