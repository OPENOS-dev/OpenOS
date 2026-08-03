#!/usr/bin/env python3
# Copyright 2023 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Update and verify cros_borealis Archlinux DB files."""

import argparse
import os
import subprocess


WORKDIR = "archlinux-databases"
CHECKSUM_FILE = "archdb_checksums"


def fetch_database_files():
    """Extract the stored database files from cros_borealis via docker cp."""
    print("Creating transient container...")
    result = subprocess.check_output(["docker", "create", "cros_borealis"])
    container_id = result.decode("utf-8").strip()

    print(f"  Copying Archlinux database files to {WORKDIR}...")
    subprocess.run(
        ["docker", "cp", f"{container_id}:/var/lib/pacman/sync", WORKDIR],
        check=True,
    )

    print("  Removing transient container...")
    subprocess.run(
        ["docker", "rm", container_id], stdout=subprocess.DEVNULL, check=True
    )


def cleanup_workdir():
    """Cleanup the working directory if it exists."""
    if os.path.isdir(WORKDIR):
        print(f"Cleaning up work directory {WORKDIR}...")
        subprocess.run(["rm", "-r", WORKDIR], check=True)


def update_checksums():
    """Update the checksum file using the current cros_borealis image."""
    print("Generating checksums for database files...")
    with open(CHECKSUM_FILE, "w") as fh:
        subprocess.run(
            f"sha512sum {WORKDIR}/*",
            shell=True,
            timeout=60,
            stdout=fh,
            check=True,
        )
    print("  Successfully updated checksums!")


def verify_checksums():
    """Verifies that the stored checksums match the current image.

    Verifies the checksums in our archdb_checksum file matches the database
    files from the current cros_borealis Docker container. On failure raises
    subprocess.CalledProcessError.
    """
    sha512_cmd = ["sha512sum", "--strict", "--check", CHECKSUM_FILE]
    print(" ".join(sha512_cmd) + ":")
    command = subprocess.run(sha512_cmd, check=True)
    command.check_returncode()


def main():
    """Main function."""
    # Change to script's directory for all relative read/writes
    abspath = os.path.abspath(__file__)
    dirname = os.path.dirname(abspath)
    os.chdir(dirname)

    parser = argparse.ArgumentParser(
        prog="Borealis Checksum Tool",
        description="Manages checksums for Borealis Arch database files",
        epilog="Contact pobega@ if you run into issues",
    )
    action = parser.add_mutually_exclusive_group(required=True)
    action.add_argument(
        "--update",
        action="store_true",
        help="Update the checksum using a local cros_borealis Docker image.",
    )
    action.add_argument(
        "--verify",
        action="store_true",
        help="Verify the saved checksums against the local cros_borealis image.",
    )
    args = parser.parse_args()

    cleanup_workdir()
    fetch_database_files()
    if args.update:
        update_checksums()
    if args.verify:
        verify_checksums()
    cleanup_workdir()


if __name__ == "__main__":
    main()
