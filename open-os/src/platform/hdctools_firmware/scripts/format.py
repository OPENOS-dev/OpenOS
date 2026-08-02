# Copyright 2023 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""This file will allow us to easily format the project. It can impact
any committed or staged file paths.
"""

import pathlib
import re
import subprocess


def get_root_directory() -> pathlib.Path:
    """Gets the base of the git repository.

    Returns:
        pathlib.Path: Git repository directory
    """

    file_dir = pathlib.Path(__file__).resolve().parent
    cmd = "git rev-parse --show-toplevel".split()
    path = subprocess.check_output(cmd, cwd=file_dir).decode().strip()
    return pathlib.Path(path)


ROOT_DIR = get_root_directory()


def changed_files() -> set[pathlib.Path]:
    """Get the files which have been committed or staged and unmerged.

    Returns:
        Set of pathlib.Paths with absolute paths of the artifacts.

    """

    cmd = "git diff --name-only --cached origin".split()
    files = subprocess.check_output(cmd, cwd=ROOT_DIR).decode().splitlines()
    return {ROOT_DIR / pathlib.Path(x) for x in files}


def path_matching(path: pathlib.Path, regex: str):
    """Identifies if a path matches our regex pattern.

    Calculates the relative path of a file to the git base and calculates
    the relative path. Uses the POSIX format to maximize compatibility.

    Args:
        path (pathlib.Path): Path of the file
        regex (str): Regex pattern

    Returns:
        bool: True if the relative path matches the pattern
    """

    rel_path = path.relative_to(ROOT_DIR).as_posix()
    if re.fullmatch(regex, rel_path):
        return True
    return False


def python_format(path: pathlib.Path):
    """Runs the Python black formater.

    Args:
        path (pathlib.Path): Path of the file we're formatting
    """

    cmd = ["black", f"{path}"]
    subprocess.check_output(cmd)


def zephyr_c_format(path: pathlib.Path):
    """Run clang-format with Zephyr's style.

    Runs clang-format with Zephyr's style, this is different than Chromium's
    internal C guide in a few areas but will allow easier upstreaming.

    Args:
        path (pathlib.Path): Path of the file we're formatting
    """

    config_path = ROOT_DIR / "zephyr_projects/sdk/zephyr/.clang-format"
    cmd = ["clang-format", f"--style=file:{config_path}", "-i", f"{path}"]
    print(" ".join(cmd))
    subprocess.check_output(cmd)


def run_format(path: pathlib.Path):
    """Run the formatting tool against the file.

    Identifies the correct formatting tool using the mapping table and
    runs it.

    Args:
        path (pathlib.Path): Path of the file we're formatting
    """

    format_map = {
        r".*\.py": python_format,
        r"zephyr_projects/.*\.(c|h)": zephyr_c_format,
    }
    found_tool = None
    for pattern, tool in format_map.items():
        result = path_matching(path, pattern)
        if result:
            found_tool = tool
            found_tool(path)
    print(path, found_tool)


all_paths = changed_files()
for path in sorted(all_paths):
    run_format(path)
