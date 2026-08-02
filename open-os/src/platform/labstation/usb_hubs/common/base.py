# Copyright 2024 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Base class that details the methods all usb hub commands must implement."""


class USBHubCommandsException(Exception):
    """Generic exception for USB hub commands failing."""


class USBHubCommands:
    """USB Hub Commands that must be implemented by all usb hub instances.

    Raises:
        NotImplementedError: if a virtual method is not implemented.

    """

    @staticmethod
    def is_hub_detected():
        """Return true if this hub type is detected, otherwise false."""
        raise NotImplementedError

    def power_cycle_servo(self, servo_serial_number, downtime_secs=5):
        """Use the hub controls to reboot the servo with serial number specified.

        Not all hubs can respect the downtime_seconds, it is hardware dependent.

        Args:
            servo_serial_number (string): servo serial number
            downtime_secs (int, optional): Number of seconds to wait before turning
                the power on. Defaults to 5.
        """
        raise NotImplementedError
