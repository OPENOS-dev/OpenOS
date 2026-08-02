#!/usr/bin/python3
# Copyright 2024 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Code to generate a mapping between Cambrionix USB hub ports to servo serials.

This is done by switching on all of the servos connected to the port then switching
them off one by one and noting which serial number was missing.

This is faster than switching them on one by one as each servo needs time to boot and
the USB device enumerated by the kernel.  This way those boots are all done in parallel.

It will fail to find any completly non responsive servo or servos that do not correctly
report their serial number in the USB enumeration.

"""

import sys
from time import sleep

import usb
from usb_hubs.cambrionix import console_lib


SERVO_PIDS = [0x520B, 0x520D]


def get_usb_serial(device, retry_count=2):
    """Call into the usb stack to get serial number.

    If the call to the USB stack fails retry up to
    retry_count times.

    Args:
        device (USBDevice): Object representing the usb device.
        retry_count (int, optional): max times to try to get the serial number.
                                     Defaults to 2.
    Returns:
        string: Serial number string or None if not retrieved.
    """
    serial_number = None
    retry = 0
    while not serial_number and retry < retry_count:
        try:
            serial_number = usb.util.get_string(device, device.iSerialNumber)
        except ValueError:
            retry += 1
            if retry < retry_count:
                print(f"Failed to get serial number for device. Retry {retry}")
                sleep(2)  # Give it a second for the usb stack to settle down.
    return serial_number


def get_all_servo_serial():
    """For each servo PID enumerate the USB devices and collect their serial number.

    Returns:
        list(string): List of connected servo serial numbers.
    """
    serialnos = []
    for product in SERVO_PIDS:
        for device in list(usb.core.find(idProduct=product, find_all=True)):
            serialnos.append(get_usb_serial(device))
    return list(filter(lambda x: x is not None, serialnos))


def reset_all(hub):
    """Reset all of the servo devices connected to the hub.

    The process of working out the mapping is destructive to servod processes anyway
    there is better success if the device are all reset.

    Args:
        hub (USBHubCommandsCambrionix): Class that wraps the serial connection to a
        specific Cambrionix hub.
    """
    hub.power_off_all()
    hub.power_on_all()
    # By experimentation 11 seconds is the minimum time it took for the 10 servos
    # typically connected to boot and enumerate.
    sleep(11)


def main():
    """Generate a yaml file with a servo serial number to usb port mapping.

    Cycle through switching off each hub port and detecting the missing
    serial number when as port is switched off.
    """

    if not console_lib.USBHubCommandsCambrionix.is_hub_detected():
        print("No single Cambrionix hub detected.")
        sys.exit(2)
    hub = console_lib.USBHubCommandsCambrionix()
    reset_all(hub)

    mapping = {}
    current_serialnos = get_all_servo_serial()
    for port in range(1, console_lib.MAX_PORT_NUMBER + 1):
        hub.power_off_port(port)
        sleep(1)
        new_serialnos = get_all_servo_serial()
        serial_no = set(current_serialnos) - set(new_serialnos)
        if serial_no:
            mapping[port] = list(serial_no)[0]
        current_serialnos = new_serialnos
    hub.power_on_all()
    hub.write_serial_to_port(mapping)


if __name__ == "__main__":
    main()
    sys.exit(0)
