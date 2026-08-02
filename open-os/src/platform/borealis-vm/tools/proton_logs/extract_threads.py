#!/usr/bin/env python3
# Copyright 2024 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Extract per-thread logs from a Proton log."""

import argparse
import os
import re


def get_thread_id(line):
    """Returns the thread id parsed from the Proton log line.

    Proton logs with a tid have two formats. The tid is always 4 chracters
    wide in hex, just like pids. Examples with tid 0178:

    # From the wine "usermode":
    # [timestamp]:[pid]:[tid]:[log_level]:[module]:[function] str
    593.322:0174:0178:trace:client:NtClose close_handle end

    # From the wine "kernel" (AKA wineserver):
    # [tid]: [function]([args...])
    0178: get_directory_entry( handle=048c, index=0000002d )

    Note: some logs (e.g. DXVK) may not have tids.
    """
    tid_extract = re.compile(
        r"""([0-9.]+:[a-f0-9]{4}:)?  # usermode ts:pid
            ([a-f0-9]{4}):           # tid
        """,
        re.VERBOSE,
    )
    tid_match = tid_extract.match(line)
    if tid_match:
        return tid_match.group(2)
    return None


def main():
    """Command-line entry-point."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("log_files", nargs="+", help="Proton log files")
    args = parser.parse_args()

    for log_file in args.log_files:
        # Build a dictionary containing the log lines for each tid.
        tid_lines = {}
        with open(log_file, encoding="utf-8", errors="backslashreplace") as log:
            for line in log.readlines():
                tid = get_thread_id(line)
                if tid is None:
                    continue
                tid_lines.setdefault(tid, []).append(line)
        # Write the log lines for each tid to its own file.
        for tid, lines in tid_lines.items():
            dirname, filename = os.path.split(log_file)
            # Save files in new directory named after the log file provided via
            # command-line suffixed with ".tid".
            out_dir = os.path.join(dirname, filename + ".tid")
            if not os.path.isdir(out_dir):
                os.mkdir(out_dir)
            tid_log_file = os.path.join(out_dir, filename + "." + tid)
            with open(tid_log_file, "w", encoding="utf-8") as tid_log:
                tid_log.writelines(lines)


if __name__ == "__main__":
    main()
