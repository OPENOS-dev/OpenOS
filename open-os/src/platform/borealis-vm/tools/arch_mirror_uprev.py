#!/usr/bin/env python3
# Copyright 2022 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Uprev Arch historic mirrors in build/pacman.d/borealis-historic-mirror.

Updates the Server URL used for obtaining database files based on the most
recent completed sync of DupIt Archlinux.

(https://ci.chromium.org/p/chromeos/builders/infra/DupIt%20Archlinux)
"""

import argparse
import os
import subprocess
import sys


BOREALIS_HISTORIC_MIRROR_FILE = "build/etc/pacman.d/borealis-historic-mirror"
MIRROR_GS_BUCKET = "chromeos-mirror"
MIRROR_BASE_HTTP_URL = f"https://storage.googleapis.com/{MIRROR_GS_BUCKET}/archlinux-archive/repos/"
LATEST_SYNC_GS_URI = (
    f"gs://{MIRROR_GS_BUCKET}/archlinux-archive/repos/latest_sync"
)


def get_borealis_dir():
    """Path to the main borealis/ directory"""
    # This script is in borealis/tools/__file__ so we want 2 levels up.
    return os.path.dirname(os.path.dirname(os.path.realpath(__file__)))


def get_borealis_historic_mirror_path():
    """Return the path to the Borealis historic mirrorlist file"""
    return os.path.join(get_borealis_dir(), BOREALIS_HISTORIC_MIRROR_FILE)


def write_file(file, contents):
    """Writes the value of 'contents' to a file using writelines().

    Args:
        file: the file to be written to
        contents: the contents of the file
    """
    with open(file, "w") as fh:
        fh.writelines(contents)


def fetch_latest_sync():
    """Returns the contents of LATEST_SYNC_GS_URI in Google Cloud Storage."""
    gsutil_cmd = f"gsutil cat {LATEST_SYNC_GS_URI}"
    output = subprocess.check_output(gsutil_cmd, shell=True)
    repo_date = output.decode(sys.stdout.encoding)
    return repo_date


def build_mirrorlist_contents(repo_date):
    """Build the Server= line for our mirrorlist file.

    Args:
        repo_date: the path date obtained from the latest_sync file
    """
    repo_url = os.path.join(MIRROR_BASE_HTTP_URL, repo_date, "$repo/os/$arch/")
    server_line = f"Server = {repo_url}"
    return server_line


def uprev_arch_mirrorlist(dry_run=False):
    """Uprev the Arch historic mirrorlist.

    Args:
        dry_run: print the contents to STDOUT instead of writing to the mirrorlist file
    """
    print(f"--> gsutil cat {LATEST_SYNC_GS_URI}")
    repo_date = fetch_latest_sync()
    print(f"----> received: {repo_date}")
    print("--> borealis_historic_mirrorlist:")
    mirrorlist_contents = build_mirrorlist_contents(repo_date)
    print(f"----> {mirrorlist_contents}")
    if dry_run:
        return
    mirrorlist_file = get_borealis_historic_mirror_path()
    print(f"--> Writing to '{mirrorlist_file}'")
    write_file(mirrorlist_file, mirrorlist_contents)
    print("----> Success")


def main():
    """Runtime entry point"""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "-n",
        "--dryrun",
        help="Do a dry run (prints the mirrorlist as it would be written to STDOUT)",
        action="store_true",
        default=False,
    )
    args = parser.parse_args()

    uprev_arch_mirrorlist(dry_run=args.dryrun)


if __name__ == "__main__":
    main()
