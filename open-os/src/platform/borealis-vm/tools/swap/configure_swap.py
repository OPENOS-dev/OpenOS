#!/usr/bin/env python3
# Copyright 2023 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Script to set up various kinds of swap memory, for tast automation."""

import argparse
import logging
import os
import stat
import subprocess
import sys
from typing import Optional


logging.basicConfig(
    format="%(asctime)s %(levelname)s %(message)s",
    datefmt="%H:%M:%S",
    level=logging.INFO,
    stream=sys.stdout,
)


def get_current_swap_devices():
    """Enumerate currently active swap devices."""
    current_devices = set()
    with open("/proc/swaps", encoding="utf-8") as f:
        f.readline()  # skip header
        for line in f:
            device_path = line.split()[0]
            # Workaround for zram, which unpredictably shows up as /zram0 and
            # /dev/zram0, but must be /dev/zram0 when used as an argument to
            # swapon/swapoff.
            if device_path == "/zram0":
                device_path = "/dev/zram0"
            current_devices.add(device_path)
    return current_devices


def set_single_swap_device(device: Optional[str]):
    """Enable a single swap device, and disable all others.

    Args:
        device: Path to a swap device, or None to disable swap entirely.
    """

    current_devices = get_current_swap_devices()

    # Enable the swap device, if one was provided, and it's not already enabled.
    if device and device not in current_devices:
        logging.info("Enabling swap device %s", device)
        subprocess.check_call(["/sbin/swapon", device])

    # Disable all other swap devices.
    for device_path in current_devices:
        if device_path != device:
            logging.info("Disabling swap device %s", device_path)
            subprocess.check_call(["/sbin/swapoff", device_path])


def disable_all_swap():
    """Disable all swap devices."""
    logging.info("Disabling all swap devices.")
    set_single_swap_device(None)


def setup_zram_swap():
    """Switch to zram swap (the default)."""
    logging.info("Switching to zram swap.")
    set_single_swap_device("/dev/zram0")


def setup_disk_swap(size=16384):
    """Set up disk swap file.

    Args:
        size: Swap file size, in MiB.
    """
    logging.info("Setting up disk swap.")

    # Switch to zram, to ensure that the swap file isn't mounted.
    setup_zram_swap()

    # Enable disk-based swap in the kernel.  This will fail on the
    # default CrOS kernel; you need to build your own kernel with
    # CONFIG_DISK_BASED_SWAP=y.
    with open("/proc/sys/vm/disk_based_swap", "w", encoding="utf-8") as f:
        f.write("1\n")

    # Create the swap file if necessary.
    swapfile = f"/mnt/stateful_partition/swap{size // 1024:d}"
    if (
        os.path.exists(swapfile)
        and os.stat(swapfile)[stat.ST_SIZE] == size * 1048576
    ):
        logging.info(
            "We already have a %d MiB swap file at %s.", size, swapfile
        )
    else:
        logging.info("Creating %d MiB swap file at %s.", size, swapfile)
        subprocess.check_call(
            [
                "/bin/dd",
                "if=/dev/zero",
                f"of={swapfile}",
                "bs=1M",
                f"count={size:d}",
            ]
        )

    # Set swap file ownership to root, change perms, and format it.
    os.chown(swapfile, 0, 0)
    os.chmod(swapfile, 0o600)
    subprocess.check_call(["/sbin/mkswap", swapfile])

    # Enable it (and disable all else.)
    set_single_swap_device(swapfile)


def main():
    """Main entrypoint."""

    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("swap_type")
    args = parser.parse_args()

    if args.swap_type == "none":
        disable_all_swap()
    elif args.swap_type == "zram":
        setup_zram_swap()
    elif args.swap_type == "disk":
        setup_disk_swap()
    else:
        raise Exception(f"Invalid swap type `{args.swap_type}`.")

    logging.info("Final swap config:")
    subprocess.check_call("/sbin/swapon")

    return 0


if __name__ == "__main__":
    sys.exit(main())
