#!/usr/bin/env python3
# Copyright 2023 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Ensure rootdev_base entry have a glob as basename:

rootdev_base contains globs that represent possible hardware paths
of the fixed SSD. They are used to find the fixed SSD to installit to
from an external device, at recovery time for instance.

The basename of these globs should not be a constant (like "nvme0n1"
instead of "nvme*n1").

The linux kernel indices for block devices may not remain constant
depending on the device initialization ordering. If for instance a
removable device is detected before the fixed device, the name of the
fixed device could become nvme1n1 sometimes.

Thus, these paths should identify devices by relatively unchanging
hardware-specific identifiers earlier in the path, while the last
component (a kernel-specific naming detail) should be a glob of some kind.
"""

import argparse
import json
import os
import re
import sys
from typing import List, Optional


def CheckRootDevBase(file_path: str) -> bool:
    """Check the metadata.rootdev_base entries have enough wildcards.

    Check the basename of the globs do have wildcard to handle variable
    indices given by the linux kernel to block devices accross boots.

    Args:
        file_path: a json file

    Returns:
        True if all the rootdevs have a wildcard in their basename.
    """
    with open(file_path, "r", encoding="utf-8") as f:
        content = f.read()

    # Remove all comments from the JSON file, otherwise the compiler
    # will not work.
    content = re.sub(r"#.*", "", content)

    data = json.loads(content)
    metadata = data.get("metadata")
    if not metadata:
        return True

    rootdev_base = metadata.get("rootdev_base")
    if not rootdev_base:
        return True

    for disk in rootdev_base.split(" "):
        if not re.search(r"[\*?]", os.path.basename(disk)):
            print(f"In {file_path}: Basename of {disk} should have a wildcard")
            return False
    return True


def CheckDiskLayoutJson(file_paths: List[str]) -> bool:
    """Check if a disk_layout.json is being modified.

    Args:
        file_paths: Files modified in this commit.

    Returns:
        True when all changes are correct.
    """
    DISK_LAYOUT_PATH = "disk_layout.json"

    for path in file_paths:
        if os.path.basename(path) == DISK_LAYOUT_PATH:
            if not CheckRootDevBase(path):
                return False
    return True


def get_parser():
    """Return an argument parser."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("files", nargs="*", help="Files to check.")
    return parser


def main(argv: Optional[List[str]] = None) -> Optional[int]:
    parser = get_parser()
    opts = parser.parse_args(argv)
    return 0 if CheckDiskLayoutJson(opts.files) else 1


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
