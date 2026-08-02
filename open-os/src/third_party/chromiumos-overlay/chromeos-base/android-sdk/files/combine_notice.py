#!/usr/bin/env python3
# Copyright 2024 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Combines license notices into one file"""

import argparse
import glob
from os import path
from pathlib import Path
import sys
from typing import List

import apache2


def list_license_files(sdk_dir: Path) -> List[Path]:
    patterns = ["**/NOTICE*", "**/*LICENSE*"]
    res = []
    for pattern in patterns:
        pattern = path.join(sdk_dir, pattern)
        res += glob.glob(pattern, recursive=True)

    return [Path(p) for p in res]


def read_file(file: Path) -> str:
    with open(file, encoding="ISO-8859-1") as f:
        return f.read()


def merge_files(files: [Path]) -> str:
    res = ""
    contents = [read_file(f) for f in files]
    contents.sort(key=len)
    contents.reverse()

    for c in contents:
        if c in res:
            continue
        res += c

    return res


def construct_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--sdk_dir",
        required=True,
        type=Path,
        help="Path to the expanded SDK directory",
    )
    parser.add_argument(
        "--output",
        required=True,
        type=Path,
        help="Path to the generate license file",
    )
    return parser


def main(argv: List[str]) -> int:
    parser = construct_parser()
    opts = parser.parse_args(argv)

    sdk_path = path.abspath(opts.sdk_dir)
    if not path.isdir(sdk_path):
        print(f"Given sdk_dir is not a directory: {sdk_path}")
        return 1

    output_file = path.abspath(opts.output)
    if not path.isdir(path.dirname(output_file)):
        print(f"Given output is not in an existing directory: {output_file}")
        return 1

    files = list_license_files(sdk_path)
    merged = merge_files(files)
    compressed = apache2.replace_apache2(merged)

    with open(output_file, mode="w", encoding="ISO-8859-1") as f:
        f.write(compressed)

    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
