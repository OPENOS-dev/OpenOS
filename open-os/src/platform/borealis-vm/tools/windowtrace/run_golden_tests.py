#!/usr/bin/env python3
# Copyright 2024 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Run "golden" integration tests for trace_window_system.py.

This script is designed to be run directly. It does not export a unittest
test case, since trace_window_system.py invokes Docker which can be slow.

The data/ folder contains sample input .tar.gz archives. This script runs
trace_window_system.py on each, and compares the output to the sample
.csv files in the same folder. Any difference triggers a failure.

To update the golden files to include intentional or harmless changes, run this
with the --update flag and commit the changed .csv files.
"""

import argparse
import os
import shutil
import subprocess
import sys
import tempfile


def find_golden_input_files():
    """Yield the names of all .tar.gz files in the data/ directory."""
    data_dir = os.path.join(os.path.dirname(sys.argv[0]), "data")
    for f in os.scandir(data_dir):
        if f.is_file() and f.name.endswith(".tar.gz"):
            yield f


def check_golden(in_name, golden_name, update):
    """Compare actual and desired output of trace_window_system.py, given the input file.

    If update is True, instead overwrite the desired output with the actual output.
    """
    out_name = None
    tools_dir = os.path.join(os.path.dirname(sys.argv[0]), "..")

    try:
        _, out_name = tempfile.mkstemp(suffix=".csv", text=True)
        subprocess.check_call(
            [
                os.path.join(tools_dir, "trace_window_system.py"),
                "--file",
                in_name,
                "--out",
                out_name,
            ]
        )
        if update:
            shutil.copyfile(out_name, golden_name)
        else:
            subprocess.check_call(["diff", "-u", golden_name, out_name])
    finally:
        if out_name:
            os.unlink(out_name)


def check_all_goldens(update):
    """Find and compare all golden files.

    Returns:
        True if golden files are found and all match; False on error, or a difference was found.
    """
    inputs = list(find_golden_input_files())
    if not inputs:
        print("No .tar.gz input files were found in the data/ folder.")
        return False

    failures = []
    for in_name in inputs:
        # Replace .tar.gz with .csv to get the name of the golden file.
        stripped = os.path.splitext(os.path.splitext(in_name)[0])[0]
        golden_name = f"{stripped}.csv"

        try:
            check_golden(in_name, golden_name, update)
        except subprocess.CalledProcessError as e:
            failures.append((in_name, e))

    if failures:
        for n, e in failures:
            print(f"{n.name}: {e}")
        print(
            f"\n{len(failures)} of {len(inputs)} windowtrace golden tests failed.\n"
        )
        print(
            "If the diffs above are desired or harmless, update the goldens by running:"
        )
        print("    tools/windowtrace/run_golden_tests.py --update")
        return False

    if update:
        print(f"\nUpdated {len(inputs)} windowtrace goldens.")
    else:
        print(f"\nAll {len(inputs)} windowtrace golden tests passed.")
    return True


def main():
    """Main entry point when run as tool/windowtrace/run_golden_tests.py."""
    parser = argparse.ArgumentParser(description=__doc__)

    parser.add_argument(
        "--update",
        default=False,
        action=argparse.BooleanOptionalAction,  # pylint: disable=no-member
        help="Write out the changed .csv files instead of failing.",
    )
    args = parser.parse_args()

    if check_all_goldens(update=args.update):
        return 0
    return 1


if __name__ == "__main__":
    sys.exit(main())
