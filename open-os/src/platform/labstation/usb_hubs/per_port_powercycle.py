#!/usr/bin/python3
# Copyright 2024 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Command line tool that given a servo serial number will power cycle the servo"""
import argparse
import sys

from usb_hubs.cambrionix import console_lib as cambrionix
from usb_hubs.plugable import console_lib as plugable


def parse_args(args):
    """Parse the command line arguments.
    Args:
        args (list[str]): The list of command line arguments passed to the script.

    Returns:
       Namespace: parsed command line arguments.
    """
    parser = argparse.ArgumentParser()
    parser.add_argument("--servo_serial", required=True)
    parser.add_argument("--downtime_secs", type=int, default=5)

    return parser.parse_args(args)


def main():
    """_summary_"""
    args = parse_args(sys.argv[1:])
    if not args.servo_serial:
        sys.exit(2)
    hub = plugable.USBHubCommandsPlugable()
    if cambrionix.USBHubCommandsCambrionix.is_hub_detected():
        hub = cambrionix.USBHubCommandsCambrionix()

    hub.power_cycle_servo(args.servo_serial, args.downtime_secs)


if __name__ == "__main__":
    main()
    sys.exit(0)
