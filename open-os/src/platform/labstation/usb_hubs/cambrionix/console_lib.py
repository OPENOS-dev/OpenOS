# Copyright 2024 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Software interface to allow power cycling ports on Cambrionix hub."""
import pathlib
from time import sleep

import serial
from usb_hubs.common import base
import yaml


CROS_TMP_DIR = "/usr/local/tmp/"
MAPPING_FILENAME = "usb_hub_mapping.yaml"
MAX_PORT_NUMBER = 15
CONSOLE_CLEAR_CHR = chr(8)
CONSOLE_CLEAR = CONSOLE_CLEAR_CHR * 20
CONSOLE_ENTER = "\r"
SERIAL_PATH = "/dev/serial/by-id/"
CAMBRIONIX_GLOB = "usb-cambrionix_SuperSync15*"


class USBHubCommandsCambrionix(base.USBHubCommands):
    """Hub interface implementation for cambrionix hubs."""

    def __init__(self):
        """Initialize the class, opening the serial console to the Cambrionix hub."""
        self.serial_to_port = None
        self.console = None
        self.usb_uartname = None
        self.get_uartname()
        self.open_console()

    def open_console(self):
        """Open the serial console to the Cambrionix hub."""
        try:
            self.console = serial.Serial(
                self.usb_uartname, baudrate=115200, write_timeout=5, timeout=5
            )
        except serial.SerialException as e:
            raise base.USBHubCommandsException(
                "Failed to connect to cambrionix console"
            ) from e

    def load_serial_to_port(self):
        """_summary_"""
        try:
            with open(
                CROS_TMP_DIR + MAPPING_FILENAME, "r", encoding="utf-8"
            ) as input_file:
                serial_to_port = yaml.safe_load(input_file)
            self.serial_to_port = {value: key for key, value in serial_to_port.items()}
        except FileNotFoundError as e:
            raise base.USBHubCommandsException("Unable to open mapping file.") from e

    def write_serial_to_port(self, serial_to_port):
        """Write out the servo serial number to port mapping as yaml.

        Args:
            serial_to_port (map{ string, string}): Mapping of servo serial number to
            cambrionix port number.
        """
        with open(
            CROS_TMP_DIR + "usb_hub_mapping.yaml", "w", encoding="utf-8"
        ) as outfile:
            yaml.dump(serial_to_port, outfile, default_flow_style=False)

    def get_port_from_serial(self, servo_serial_number):
        """Get the port number from the servo serial number.

        Args:
            servo_serial_number (string): Servo serial number.

        Returns:
            int : Cambrionix port number associated with servo serial number.
        """
        return self.serial_to_port.get(servo_serial_number)

    def send_to_console(self, command):
        """Send a command to the cambrionix console

        Args:
            command (string): cambrionix console command.
        """
        command_to_send = command + "\r"
        self.console.write(command_to_send.encode("utf-8"))

    def power_off_port(self, port):
        """Send command to power off a specific hub ports.

        Args:
            port (int): hub port number.
        """
        self.send_to_console(f"mode off {port}")

    def power_on_port(self, port):
        """Send command to power on a specific hub ports.

        Args:
            port (int): hub port number.
        """
        self.send_to_console(f"mode sync {port}")

    def power_on_all(self):
        """Send command to power on all hub ports."""
        self.send_to_console("mode sync")

    def power_off_all(self):
        """Send command to power off all hub ports."""
        self.send_to_console("mode off")

    def get_uartname(self):
        """Search for the serial devices that matches the Cambrionix pattern and open
        the first device.
        """
        usb_uartnames = [
            str(name) for name in pathlib.Path(SERIAL_PATH).glob(CAMBRIONIX_GLOB)
        ]
        self.usb_uartname = usb_uartnames[0]

    @staticmethod
    def is_hub_detected():
        """Search that to find the number of serial devices attached that match
           the Cambrionix pattern.

        Returns:
            bool: True if one and only one cambrionix hub is detected.
        """
        usb_uartnames = list(pathlib.Path(SERIAL_PATH).glob(CAMBRIONIX_GLOB))
        if len(usb_uartnames) > 1:
            print("More than 1 cambrionix hub found panic !!")
        return len(usb_uartnames) == 1

    def power_cycle_servo(self, servo_serial_number, downtime_secs=5):
        """Given a servo serial number power cycle the correct cambrionix port number.

        Args:
            servo_serial_number (string): serial number of a servo attached to the hub.
            downtime_secs (int, optional): _description_. Defaults to 5.
        """
        self.load_serial_to_port()
        port = self.get_port_from_serial(servo_serial_number)
        self.power_off_port(port)
        sleep(downtime_secs)
        self.power_on_port(port)
