# Copyright 2024 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# pylint: disable=redefined-outer-name
# pylint: disable=unused-argument
# pylint: disable=import-error
# pylint: disable=missing-module-docstring
# pylint: disable=missing-function-docstring
# pylint: disable=missing-class-docstring
# pylint: disable=R0801
# When pylint supports proto better remove
# pylint: disable=no-member

import sys

import pytest
from usb_hubs.per_port_powercycle import main


class TestPortCycle:
    def test_no_servo_serial_flag(
        self, monkeypatch, mock_host_with_one_cambrionix, capfd
    ):
        mock_host_with_one_cambrionix()

        with monkeypatch.context() as patch:
            patch.setattr(sys, "argv", ["per_port_powercycle"])

            with pytest.raises(SystemExit) as pytest_wrapped_e:
                main()
            out, err = capfd.readouterr()
            assert (
                "usage: per_port_powercycle [-h] --servo_serial SERVO_SERIAL\n"
                "                           [--downtime_secs DOWNTIME_SECS]\n"
                "per_port_powercycle: error: the following arguments are required: --servo_serial\n"
            ) == err
            assert SystemExit == pytest_wrapped_e.type

    def test_basic(self, monkeypatch, mock_host_with_one_cambrionix, capfd):
        host = mock_host_with_one_cambrionix()

        with monkeypatch.context() as patch:
            patch.setattr(sys, "argv", ["per_port_powercycle", "--servo_serial", "a"])
            main()
            out, unused_err = capfd.readouterr()
            assert "" == out
            assert b"mode off 1\rmode sync 1\r" == host.console.written

        host.console.written = b""
        with monkeypatch.context() as patch:
            patch.setattr(sys, "argv", ["per_port_powercycle", "--servo_serial", "o"])
            main()
            out, unused_err = capfd.readouterr()
            assert "" == out
            assert b"mode off 15\rmode sync 15\r" == host.console.written
