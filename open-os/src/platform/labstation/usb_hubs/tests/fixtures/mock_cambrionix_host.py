# Copyright 2024 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# pylint: disable=unused-argument
# pylint: disable=import-error
# pylint: disable=missing-module-docstring
# pylint: disable=missing-function-docstring
# pylint: disable=missing-class-docstring
# pylint: disable=import-error
# pylint: disable=no-name-in-module


import logging
import os
from pathlib import PosixPath
import re

import pytest
import serial
from usb_hubs.cambrionix.console_lib import SERIAL_PATH


_logger = logging.getLogger("mock_cambrionix_host")


class USBDevice:
    iSerialNumber = None
    pid = None

    def __init__(self, serial_number, pid):
        self.iSerialNumber = serial_number
        self.pid = pid


@pytest.fixture(scope="function")
def mock_cambrionix_host(class_mocker, mocker):
    def generate_cambrionix_host():
        """Mock generator function.   This allows multiple tests to be run in
        parallel as it generates a new mock for each test vs sharing the same
        mock between tests.

        Returns:
            MockCambrionixHost:
        """

        class MockCambrionixHost:
            def __init__(self):
                self.usb_devices = []
                class_mocker.patch("serial.Serial", side_effect=self.mock_serial_open)
                class_mocker.patch(
                    "pathlib.PosixPath.glob",
                    side_effect=self.mock_glob,
                )
                class_mocker.patch(
                    "usb.core.find",
                    side_effect=self.mock_usbfind,
                )

                class_mocker.patch(
                    "usb.util.get_string",
                    side_effect=self.mock_usb_get_string,
                )

                mocker.patch(
                    "time.sleep",
                    side_effect=self.mock_sleep,
                )

                self.mock_open = mocker.mock_open(
                    read_data=(
                        "1: a\n2: b\n3: c\n4: d\n5: e\n6: f\n7: g\n8: h\n9: i\n10:"
                        " j\n11: k\n12: l\n13: m\n14: n\n15: o\n"
                    )
                )

                mocker.patch("builtins.open", self.mock_open)

                self.port_device_mapping = {
                    1: USBDevice("a", 0x520B),
                    2: USBDevice("b", 0x520B),
                    3: USBDevice("c", 0x520B),
                    4: USBDevice("d", 0x520B),
                    5: USBDevice("e", 0x520B),
                    6: USBDevice("f", 0x520B),
                    7: USBDevice("g", 0x520B),
                    8: USBDevice("h", 0x520B),
                    9: USBDevice("i", 0x520B),
                    10: USBDevice("j", 0x520B),
                    11: USBDevice("k", 0x520B),
                    12: USBDevice("l", 0x520B),
                    13: USBDevice("m", 0x520B),
                    14: USBDevice("n", 0x520B),
                    15: USBDevice("o", 0x520B),
                }

                self.console = None

            def set_usb_devices(self, usb_devices):
                self.usb_devices = usb_devices

            def clear_usb_devices(self):
                self.usb_devices = []

            def raise_on_open(self):
                class_mocker.patch(
                    "serial.Serial", side_effect=serial.SerialException()
                )

            def mock_glob(self, glob_str):
                matched_usb_devices = [
                    device for device in self.usb_devices if re.match(glob_str, device)
                ]
                return [
                    PosixPath(f"{SERIAL_PATH}{device}")
                    for device in matched_usb_devices
                ]

            # pylint
            def mock_serial_open(self, port, baudrate, write_timeout, timeout):
                if not os.path.basename(port) in self.usb_devices:
                    raise serial.SerialException(f"Device not found {port}.")
                self.console = self.usb_devices[os.path.basename(port)]
                return self.console

            def mock_usbfind(self, idProduct, find_all):
                results = []
                for port, enabled in self.console.port_enabled_mapping.items():
                    if enabled and self.port_device_mapping[port].pid == idProduct:
                        results.append(self.port_device_mapping[port])
                return results

            def mock_usb_get_string(self, device, attr):
                return attr

            def mock_sleep(self, time):
                return

        return MockCambrionixHost()

    return generate_cambrionix_host
