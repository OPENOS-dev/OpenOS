# Copyright 2023 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Firmware updater for Starfish

Firmware updater for Starfish. This requires dfu-util and pyusb and can be
run from a virtual environment:

python3 -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt

This is currently a WIP intended for the initial builds:
"""


import argparse
import code
import enum
import pathlib
import subprocess
import sys
import time
import usb


class DeviceDFUMode(enum.Enum):
    DFU_UNKNOWN = 0
    DFU_RUNTIME = 1
    DFU_BOOTLOADER = 2


def stm_dfu_device(dev):
    """Matches STM32 DFU devices"""
    return dev.idVendor == 0x0483 and dev.idProduct == 0xDF11


def starfish_device(dev):
    """Matches Starfish devices"""
    return dev.idVendor == 0x18D1 and dev.idProduct == 0x5060


def get_string(dev, index):
    """Finds USB String at a given index."""
    if index != 0:
        text = usb.util.get_string(dev, index)
        return text
    return None


def find_serial_string(dev):
    """Finds USB Serial String."""
    return get_string(dev, dev.iSerialNumber)


def check_runtime(interface):
    """Check which state the device is in."""
    if interface.bInterfaceProtocol == 0x01:
        return DeviceDFUMode.DFU_RUNTIME
    if interface.bInterfaceProtocol == 0x02:
        return DeviceDFUMode.DFU_BOOTLOADER
    raise DeviceDFUMode.DFU_UNKNOWN


def find_dfu_descriptors(dev):
    """Finds the dfu descriptor when version matching is supported."""
    all_interfaces = []
    code.interact(local=locals())
    for config in dev:
        for x in config:
            if x.bInterfaceClass == 0xFE and x.bInterfaceSubClass == 0x01:
                # We have a valid DFU device
                mode = check_runtime(x)
                text = get_string(dev, x.iInterface)
                all_interfaces.append((mode, text))
    print(all_interfaces)
    return all_interfaces


def list_starfish():
    """List all Starfish devices."""
    query = lambda x: x.idVendor == 0x18D1 and x.idProduct == 0x5060
    gen = usb.core.find(custom_match=query, find_all=True)
    return list(gen)


def get_usb_path(device):
    """Gets the USB Device path so dfu-util can handle multiple devices."""
    port_nums = ".".join([str(x) for x in device.port_numbers])
    return f"{device.bus}-{port_nums}"


def call_dfu(file_path, dev):
    """Call the DFU utility on the specific USB device."""
    vid_pid = f"{hex(device.idVendor)[2:]}:{hex(device.idProduct)[2:]}"
    usb_path = get_usb_path(device)
    dfu_cmd = ["sudo", "dfu-util"]
    usb_args = ["-d", vid_pid, "-p", usb_path]
    flash_args = ["-a", "0", "-s", "0x8000000:leave"]
    file_args = ["-D", str(file_path)]
    cmd = dfu_cmd + usb_args + flash_args + file_args
    print(" ".join(cmd))
    subprocess.call(cmd)


def wait_for_count(target_count: int, timeout: float):
    """Wait for the number of USB devices to reach a target count
    and return the list of devices.

    Args:
        target_count (int): Target number
        timeout (float): Timeout in seconds

    Returns:
        List of USB devices

    Raises:
        Exception: Exception on timeout expiring
    """

    stop = time.time() + timeout
    last_device_count = None

    while True:
        devices = list_starfish()
        device_count = len(devices)
        if device_count == target_count:
            return devices

        if last_device_count != device_count:
            last_device_count = device_count
            print(f"{device_count} devices found.")

        if time.time() > stop:
            raise Exception("Timeout waiting")
        time.sleep(0.1)


def fw_update(path: pathlib.Path, loop: bool):
    """Updates the firmware on devices

    Args:
        path (pathlib.Path): Path of DFU file
        loop (bool): Enables looping
    """

    while True:
        print("Waiting for the device")
        devices = wait_for_count(1, 60)
        call_dfu(path, devices[0])

        print("Waiting for device to return")
        # We might accidentally see the STM32 devices during the reboots
        # cycles if we scan too quickly.
        time.sleep(1)
        wait_for_count(1, 60)
        print("Found the device again")

        if not loop:
            break

        print("Waiting for disconnection")
        wait_for_count(0, 60)


def main():
    parser = argparse.ArgumentParser(allow_abbrev=False)
    parser.add_argument("path", type=pathlib.Path)
    parser.add_argument("--loop", action="store_true")
    args = parser.parse_args()

    if not args.path.is_file():
        raise Exception(f"{args.path} does not exist")

    fw_update(args.path, args.loop)
    return 0


if __name__ == "__main__":
    sys.exit(main())
