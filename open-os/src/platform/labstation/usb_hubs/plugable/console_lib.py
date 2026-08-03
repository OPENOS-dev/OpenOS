# Copyright 2024 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Software interface to allow power cycling ports on plugable hubs."""
import logging
import os
import subprocess
import sys
import time

from usb_hubs.common import usb_hierarchy as uh
from usb_hubs.common import base


# List of PID supported for power-cycle. For now, it's only v4 and v4p1.
# 0x520d: v4p1, 0x501b: v4
PWR_CYCLE_PIDS = [0x520D, 0x501B]

# VID to find all servo devices.
SERVO_VID = 0x18D1


class USBHubCommandsPlugable(base.USBHubCommands):
    """Hub interface implementation for plugable hubs."""

    # Lookup table for action on uhubctl.
    ACTION_DICT = {"on": 1, "off": 0, "reset": 2}

    # Repetitions to perform on the uhubctl command to get it to trigger.
    REPS = 100

    # Time to sleep after reset to let kernel handle sysfs files
    RESET_DEBOUNCE_S = 2

    # time to sleep for the servo device to re-enumerate.
    MAX_REINIT_SLEEP_S = 8

    # Polling intervals to find the |devnum| file for reset device.
    REINIT_POLL_SLEEP_S = 0.1

    # Time to sleep and attempts to interact with servo console after reboot.
    REBOOT_SLEEP_S = 1
    REBOOT_TIMEOUT_ATTEMPTS = 4

    # Time to sleep after power off
    PWR_OFF_SLEEP_S = 1

    @staticmethod
    def is_hub_detected():
        # By default the fleet always assume that a plugable hub is connected.
        # TODO can try searching for a VID/PID but it would not be 100% successful.
        return True

    def __init__(self):
        """Setup scratch to use."""
        self._logger = logging.getLogger(type(self).__name__)

    def error(self, msg, *args):
        # pylint: disable=invalid-name
        """Log error and exit.

        Args:
          msg: message to log
          *args: args to pass to logging module
        """
        self._logger.info(msg, *args)
        sys.exit(1)

    def _usb_path(self, serial):
        """Helper to get the device path.

        Args:
          serial: str, servo serial

        Returns:
          /sys/bus/usb/devices/ path to servo with |serial| or None if not found
        """
        # This list is used to find all servos on the system.
        vid_pid_list = [(SERVO_VID, None)]
        devs = uh.get_all_usb_device_sysfs_paths(vid_pid_list)
        for dev_path in devs:
            dev_serial = uh.serial_from_sysfs(dev_path)
            if dev_serial == serial:
                return dev_path
        return None

    def _check_devnum_reset(self, dev_path, devnum, action):
        """Check that the |devnum| has changed after a reset/reboot/power-cycle

        Args:
          dev_path: device sysfs path
          devnum: int, usb devnum (original devnum, before reset action)
          action: str, action performed (used to print better errors/logs

        Note: this helper will call self.error() (and thus exit) if
        - the devnum does not change
        - it fails to read the devnum after self.MAX_REINIT_SLEEP_S
        """
        # Sleep a bit to let the device fully fall off, and the sysfs files be
        # renewed.
        time.sleep(self.RESET_DEBOUNCE_S)
        # For |MAX_REINIT_SLEEP_S| seconds, try to find the new devnum for the
        # device.
        end = time.time() + self.MAX_REINIT_SLEEP_S
        while time.time() < end:
            try:
                # check devnum reset
                if devnum == uh.dev_num_from_sysfs(dev_path):
                    self.error(
                        "%r likely unsuccessful. devnum stayed the same.", action
                    )
                # If |devnum| changed, then the goal is fulfilled. Move on.
                break
            except uh.HierarchyError:
                # The device might not have re-enumerated yet. Sample again.
                time.sleep(self.REINIT_POLL_SLEEP_S)
        else:
            # The while loop finished without breaking out e.g. we never read the
            # |devnum| file successfully.
            self.error(
                "unable to read device |devnum| file after %ds. Giving up.",
                self.MAX_REINIT_SLEEP_S,
            )

    def _run_uhubctl_command(self, hub, port, action):
        """Build |uhubctl| command performing |action| on |hub|'s |port|.

        Args:
          hub: hub-port path i.e. /sys/bus/usb/devices/ dirname of the hub
          port: str, port number on the hub
          action: one of 'on', 'off', 'reset'
        """
        cmd = self._build_and_assert_uhubctl(hub=hub, port=port)
        if action not in self.ACTION_DICT:
            self.error("Action %s unknown", action)
        action_number = self.ACTION_DICT[action]
        # expand command to perform the uhubctl action.
        cmd = cmd + ["-a", str(action_number), "-r", str(self.REPS)]
        try:
            subprocess.check_output(cmd, stderr=subprocess.STDOUT)
        except subprocess.CalledProcessError as e:
            self.error(
                'Error performing the uhubctl command. ran: "%s". %s.',
                " ".join(cmd),
                str(e),
            )

    def _build_and_assert_uhubctl(self, hub=None, port=None):
        """Assert uhubctl exists and hub/port are known to it (if provided).

        Args:
          hub: hub-port path i.e. /sys/bus/usb/devices/ dirname of the hub
          port: str, port number on the hub

        Returns:
          cmd: a list of args to call the uhubctl command (at hub/port if provided)
        """
        cmd = ["sudo", "uhubctl"]
        try:
            subprocess.check_output(cmd, stderr=subprocess.STDOUT)
        except subprocess.CalledProcessError as e:
            self.error("uhubctl not available. Be sure to run as sudo. %s", str(e))
        if hub is not None and port is not None:
            # expand the command to check for hub and port existing.
            # casting port to str to ensure we don't accidentally pass an int.
            cmd = cmd + ["-l", hub, "-p", str(port)]
            try:
                subprocess.check_output(cmd, stderr=subprocess.STDOUT)
            except subprocess.CalledProcessError as e:
                self.error(
                    "hub %s with port %s unknown to uhubctl. "
                    "Be sure the hub is a supported smart hub. %s",
                    hub,
                    port,
                    str(e),
                )
        return cmd

    def _get_hub_and_port(self, dev_path, pid):
        """Get the external hub and port paths for a given |pid|.

        This is its own function, as different devices might have a different
        internal topology. This method has to guarantee to implement all pids
        in PWR_CYCLE_PIDS.

        Args:
          dev_path: /sys/bus/usb/devices/ path for servo
          pid: servo pid

        Returns:
          (hub, port, hub3,) tuple, where hub is the external hub that the
                             servo is on port is the port on that hub that the
                             servo is on. hub3 is the usb3 virtual hub of the same
                             physical hub. It will be None if it does not exist
                             (hub only enumerated in usb2)
        """
        # NOTE: if a servo device or configuration should be supported, this is the
        # spot to implement it. If more devices get implemented here, make sure to
        # document the setup under which the user can expect it to work and for the
        # code to guard against other setups.
        # For example: if a servo micro is attached to a servo v4, the code to reset
        # the micro should not just reset the hub that the v4 is hanging on, as that
        # would reset both.
        # TODO(coconutruben): make pid permission more robust once we have device
        # templates.
        if pid in PWR_CYCLE_PIDS:
            # For servo v4(p1), the dev_path points to the stm that's hanging on
            # an internal usb hub.
            internal_hub = uh.get_sysfs_parent_hub_stub(dev_path)
            smart_hub_path = uh.get_sysfs_parent_hub_stub(internal_hub)
            if smart_hub_path:
                # The internal hub is hanging on the smart hub's port. So the last
                # index is the port number.
                port = internal_hub.rsplit(".", 1)[-1]
                smart_hub = os.path.basename(smart_hub_path)
                # |internal_hub| is always on usb2. Let's see if this hub also
                # enumerated on usb3.
                smart_hub_bus, unused_ = smart_hub.split("-")
                busnum = int(smart_hub_bus)
                busnum3 = uh.complement_bus_num(busnum)
                smart_hub3 = None
                if busnum3:
                    smart_hub3 = "busnum3-smart_hub_port_path"
                    smart_hub3_path = os.path.join(
                        os.path.dirname(smart_hub_path), smart_hub3
                    )
                    if not os.path.exists(smart_hub3_path):
                        # set back to None
                        smart_hub3 = None
                return (smart_hub, port, smart_hub3)
            self.error(
                "Device does not seem to be hanging on a (smart) hub. %r", dev_path
            )

        self.error("Unimplemented pid: %04x", pid)

    def power_cycle_servo(self, servo_serial_number, downtime_secs=5):
        """Perform a power-cycle on the device using uhubctl.

        uhubctl exposes multiple knobs to control the power-cycling of a port.
        This method uses the 'reset' knob by default. However, if the user
        specifies |force|=True, it will issue an 'off' request, wait for
        |PWR_OFF_SLEEP_S| seconds, before issuing an 'on' request. For some hubs
        this has proven itself more reliably than a reset request.

        Args:
          force: bool, whether to perform a full power-cycle or just a reset
        """
        self._build_and_assert_uhubctl()
        dev_path = self._usb_path(servo_serial_number)
        if not dev_path:
            self.error("Device with serial %r not found.", servo_serial_number)
        pid = uh.product_id_from_sysfs(dev_path)
        if pid not in PWR_CYCLE_PIDS:
            self.error(
                "pid: 0x%04x currently not supported for usb power cycling. "
                "Please use one of: %s",
                pid,
                ", ".join("0x%04x" % p for p in PWR_CYCLE_PIDS),
            )
        # get devnum, and store it
        devnum = uh.dev_num_from_sysfs(dev_path)
        # extract the hub and check whether it's on uhubctl
        hub, port, hub3 = self._get_hub_and_port(dev_path, pid)

        # The sandwich (if usb2 and usb3 are available) is to first turn
        # off usb2 and then usb3, before unrolling that operation.
        self._run_uhubctl_command(hub=hub, port=port, action="off")
        if hub3 is not None:
            self._run_uhubctl_command(hub=hub3, port=port, action="off")
        time.sleep(self.PWR_OFF_SLEEP_S)
        if hub3 is not None:
            self._run_uhubctl_command(hub=hub3, port=port, action="on")
        self._run_uhubctl_command(hub=hub, port=port, action="on")

        self._check_devnum_reset(dev_path, devnum, "power-cycle")
        # At the end, no error was encountered, so indicate belief that reset was
        # successful.
        self._logger.info(
            "Successfully power-cycled device with serial %r. "
            "(At least reasonably confident).",
            servo_serial_number,
        )
