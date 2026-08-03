#!/usr/bin/env python3
# Copyright 2022 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Uprev PKGBUILDs in build/packages/.

Updates the commit hash pointed to for the source of PKGBUILDs using regular
expressions. Commit hashes are obtained via 'git ls-remote'.
"""

import argparse
import glob
import os
import re
from shutil import copy
import subprocess
import sys
import tempfile


# Regex to match the _borealis_branch_pin variable in a capture group.
PINNED_BRANCH_REGEX = re.compile(
    r"^_borealis_branch_tracked=\([\'\"](?P<branch>.+)[\'\"]\)"
)
# Regex to extract the Git repository URL from the 'source=' line using a capture group
# Example of a source line:
# source=("apitrace::git+https://source.com/apitrace#${_borealis_current_ref}")
# This regex extracts 'https://source.com/apitrace' into the url capture group
GIT_URL_REGEX = re.compile(r"^source=\(.*git\+(?P<url>.*?)#.*")


def get_borealis_dir():
    """Path to the main borealis/ directory"""
    # This script is in borealis/tools/__file__ so we want 2 levels up.
    return os.path.dirname(os.path.dirname(os.path.realpath(__file__)))


def get_pkgbuild_list():
    """Return a list of all PKGBUILDs in borealis/build/packages/*"""
    return glob.glob(
        os.path.join(get_borealis_dir(), "build/packages/*/PKGBUILD")
    )


def get_regex_match(file, regex):
    """Read a file line by line, returning once a regex match succeeds.

    Args:
        file: file to be read until the regex matches
        regex: regex to match each line against
    """
    with open(file, "r") as fh:
        for line in fh:
            match = regex.match(line)
            if match:
                return match
    return None


def get_pkgbuild_pinned_branch(pkgbuild_path):
    """Return the pinned branch name for a given PKGBUILD.

    Args:
        pkgbuild_path: path to the PKGBUILD to get the pinned branch from
    """
    match = get_regex_match(pkgbuild_path, PINNED_BRANCH_REGEX)
    if match:
        return match.group(1)
    return None


def get_pkgbuild_git_repo_url(pkgbuild_path):
    """Extract the source's Git URL from a PKGBUILD.

    Args:
        pkgbuild_path: path to the PKGBUILD to get the Git URL from
    """
    match = get_regex_match(pkgbuild_path, GIT_URL_REGEX)
    if match:
        return match.group(1)
    return None


def git_ls_remote(git_url, branch):
    """Get the commit hash of a branch from a git remote.

    Args:
        git_url: git repository URL to get ref from
        branch: git branch we want the HEAD commit hash from
    """
    git_cmd = f"git ls-remote {git_url} refs/heads/{branch}"
    output = subprocess.check_output(git_cmd, shell=True)
    commit_hash = output.decode(sys.stdout.encoding).split("\t")[0]
    return commit_hash


def generate_new_pkgbuild(pkgbuild, commit):
    """Generate a new PKGBUILD with an updated commit hash.

    Reads the current PKGBUILD line by line until the _borealis_current_ref
    variable is found. Replaces that line with one pointing at the commit hash
    provided to this function.

    Returns the contents of the new PKGBUILD as a list of strings.

    Args:
        pkgbuild: path to the PKGBUILD to read from.
        commit: the commit hash to uprev _borealis_current_ref to
    """
    new_contents = []
    # Read the PKGBUILD line by line. When the source= line is encountered
    # append a new source line in its place using the provided commit hash.
    with open(pkgbuild, "r") as pkgbuild_fh:
        for pkgbuild_line in pkgbuild_fh:
            if pkgbuild_line.lstrip().startswith("_borealis_current_ref="):
                new_contents.append(
                    f"_borealis_current_ref=('commit={commit}')\n"
                )
            else:
                new_contents.append(pkgbuild_line)

    return new_contents


def update_pkgbuild_atomic(pkgbuild, contents):
    """Atomically write out a PKGBUILD file.

    Writes to a temp file first before copying that file in place over the
    target PKGBUILD.

    Args:
        pkgbuild: path to the PKGBUILD to overwrite
        contents: complete file contents of the new PKGBUILD as a list
    """
    # Write the new PKGBUILD out into a tempfile
    with tempfile.NamedTemporaryFile() as tempf:
        tmpfile = tempf.name
    with open(tmpfile, "w") as tmpfh:
        tmpfh.writelines(contents)

    # Copy our new file to an intermediate file in the same PATH as the target
    # PKGBUILD, then do an atomic delete & rename (using os.replace) to
    # overwrite the original PKGBUILD with our new one.
    intermediate_file = os.path.join(
        os.path.dirname(pkgbuild), os.path.basename(tmpfile)
    )
    copy(tmpfile, intermediate_file)
    os.replace(intermediate_file, pkgbuild)


def uprev_pkgbuild(pkgbuild_path, commit_hash):
    """Uprev a PKGBUILD.

    Uprev the source= entry in a given PKGBUILD.

    Args:
        pkgbuild_path: path to the PKGBUILD to uprev
        commit_hash: commit hash to uprev to
    """
    new_pkgbuild_contents = generate_new_pkgbuild(pkgbuild_path, commit_hash)
    update_pkgbuild_atomic(pkgbuild_path, new_pkgbuild_contents)


def main():
    """Runtime entry point"""
    parser = argparse.ArgumentParser(description=__doc__)

    parser.add_argument(
        "--all",
        help="Attempt to uprev all PKGBUILDs in borealis/build/packages/",
        action="store_true",
        default=False,
    )
    parser.add_argument(
        "pkgbuilds", nargs="*", metavar="PKGBUILD", default=None
    )

    args = parser.parse_args()

    if args.all and args.pkgbuilds:
        print("--all and PKGBUILDs are mutually exclusive options.")
        sys.exit(1)
    if args.all:
        pkgbuilds = get_pkgbuild_list()
    elif args.pkgbuilds:
        pkgbuilds = args.pkgbuilds
    else:
        parser.print_help()
        sys.exit(1)

    for pkgbuild in pkgbuilds:
        print(f"Checking {pkgbuild}")
        branch = get_pkgbuild_pinned_branch(pkgbuild)
        if branch:
            print("--> Upreving package")
            git_url = get_pkgbuild_git_repo_url(pkgbuild)
            print(f"----> git remote: {git_url}")
            commit_hash = git_ls_remote(git_url, branch)
            print(f"----> hash at origin/{branch}: {commit_hash}")
            uprev_pkgbuild(pkgbuild, commit_hash)
        else:
            print("nothing to do")
        print()


if __name__ == "__main__":
    main()
