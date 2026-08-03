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
import time
from unittest.mock import call
from unittest.mock import MagicMock
from unittest.mock import patch

import pytest
from usb_hubs.cambrionix.find_mapping import get_all_servo_serial
from usb_hubs.cambrionix.find_mapping import main


class TestFindMapping:
    def test_find_mapping_no_cambrionix(
        self, monkeypatch, mock_host_with_no_cambrionix, capfd
    ):
        mock_host_with_no_cambrionix()

        with monkeypatch.context() as patch:
            patch.setattr(sys, "argv", ["find-cambrionix-mapping"])

            with pytest.raises(SystemExit) as pytest_wrapped_e:
                main()
            out, unused_err = capfd.readouterr()
            assert ("No single Cambrionix hub detected.\n") == out
            assert SystemExit == pytest_wrapped_e.type

    def test_find_mapping_cambrionix(
        self, monkeypatch, mock_host_with_one_cambrionix, capfd
    ):
        host = mock_host_with_one_cambrionix()

        with monkeypatch.context() as patch:
            patch.setattr(sys, "argv", ["find-cambrionix-mapping"])
            main()
            out, unused_err = capfd.readouterr()
            host.mock_open.assert_called_once_with(
                "/usr/local/tmp/usb_hub_mapping.yaml", "w", encoding="utf-8"
            )
            write_calls = [
                call("1"),
                call(":"),
                call(" "),
                call("a"),
                call("\n"),
                call("2"),
                call(":"),
                call(" "),
                call("b"),
                call("\n"),
                call("3"),
                call(":"),
                call(" "),
                call("c"),
                call("\n"),
                call("4"),
                call(":"),
                call(" "),
                call("d"),
                call("\n"),
                call("5"),
                call(":"),
                call(" "),
                call("e"),
                call("\n"),
                call("6"),
                call(":"),
                call(" "),
                call("f"),
                call("\n"),
                call("7"),
                call(":"),
                call(" "),
                call("g"),
                call("\n"),
                call("8"),
                call(":"),
                call(" "),
                call("h"),
                call("\n"),
                call("9"),
                call(":"),
                call(" "),
                call("i"),
                call("\n"),
                call("10"),
                call(":"),
                call(" "),
                call("j"),
                call("\n"),
                call("11"),
                call(":"),
                call(" "),
                call("k"),
                call("\n"),
                call("12"),
                call(":"),
                call(" "),
                call("l"),
                call("\n"),
                call("13"),
                call(":"),
                call(" "),
                call("m"),
                call("\n"),
                call("14"),
                call(":"),
                call(" "),
                call("n"),
                call("\n"),
                call("15"),
                call(":"),
                call(" "),
                call("o"),
                call("\n"),
            ]
            host.mock_open().write.assert_has_calls(calls=write_calls, any_order=False)
            assert ("") == out

    def test_find_mapping_multiple_cambrionix(
        self, monkeypatch, mock_host_with_multiple_cambrionix, capfd
    ):
        mock_host_with_multiple_cambrionix()

        with monkeypatch.context() as patch:
            patch.setattr(sys, "argv", ["find-cambrionix-mapping"])

            with pytest.raises(SystemExit) as pytest_wrapped_e:
                main()
            out, unused_err = capfd.readouterr()
            assert (
                "More than 1 cambrionix hub found panic !!\nNo single Cambrionix hub detected.\n"
            ) == out
            assert SystemExit == pytest_wrapped_e.type


class TestGetAllServoSerial:
    @patch("usb_hubs.cambrionix.find_mapping.usb.core.find")
    @patch("usb_hubs.cambrionix.find_mapping.usb.util.get_string")
    def test_get_all_servo_serial_success(self, mock_get_string, mock_find):
        # Mock devices
        mock_dev1 = MagicMock(iSerialNumber=1)
        mock_dev2 = MagicMock(iSerialNumber=2)
        mock_dev3 = MagicMock(iSerialNumber=3)

        # Configure mock_find to return different devices for different PIDs
        mock_find.side_effect = [
            [mock_dev1, mock_dev2],  # For PID 0x520B
            [mock_dev3],  # For PID 0x520D
        ]

        # Configure mock_get_string
        mock_get_string.side_effect = ["serial1", "serial2", "serial3"]

        result = get_all_servo_serial()

        assert result == ["serial1", "serial2", "serial3"]
        mock_find.assert_has_calls(
            [
                call(idProduct=0x520B, find_all=True),
                call(idProduct=0x520D, find_all=True),
            ]
        )
        mock_get_string.assert_has_calls(
            [
                call(mock_dev1, mock_dev1.iSerialNumber),
                call(mock_dev2, mock_dev2.iSerialNumber),
                call(mock_dev3, mock_dev3.iSerialNumber),
            ]
        )

    @patch("usb_hubs.cambrionix.find_mapping.usb.core.find")
    def test_get_all_servo_serial_no_devices(self, mock_find):
        mock_find.return_value = []
        result = get_all_servo_serial()
        assert result == []
        mock_find.assert_has_calls(
            [
                call(idProduct=0x520B, find_all=True),
                call(idProduct=0x520D, find_all=True),
            ]
        )

    @patch("usb_hubs.cambrionix.find_mapping.usb.core.find")
    @patch("usb_hubs.cambrionix.find_mapping.usb.util.get_string")
    def test_get_all_servo_serial_one_pid_found(self, mock_get_string, mock_find):
        mock_dev1 = MagicMock(iSerialNumber=1)
        mock_find.side_effect = [[mock_dev1], []]  # Only PID 0x520B found
        mock_get_string.return_value = "serial1"

        result = get_all_servo_serial()

        assert result == ["serial1"]
        mock_find.assert_has_calls(
            [
                call(idProduct=0x520B, find_all=True),
                call(idProduct=0x520D, find_all=True),
            ]
        )
        mock_get_string.assert_called_once_with(mock_dev1, mock_dev1.iSerialNumber)

    @patch("usb_hubs.cambrionix.find_mapping.usb.core.find")
    @patch("usb_hubs.cambrionix.find_mapping.usb.util.get_string")
    @patch("usb_hubs.cambrionix.find_mapping.sleep", return_value=None)  # Mock sleep
    def test_get_all_servo_serial_get_string_fails_once(
        self, mock_sleep, mock_get_string, mock_find
    ):
        mock_dev1 = MagicMock(iSerialNumber=1)
        mock_find.side_effect = [[mock_dev1], []]
        # Fail first time, succeed second time
        mock_get_string.side_effect = [ValueError("USB Error"), "serial1_retry"]

        result = get_all_servo_serial()

        assert result == ["serial1_retry"]
        mock_sleep.assert_called_once_with(2)
        assert mock_get_string.call_count == 2

    @patch("usb_hubs.cambrionix.find_mapping.usb.core.find")
    @patch("usb_hubs.cambrionix.find_mapping.usb.util.get_string")
    @patch("usb_hubs.cambrionix.find_mapping.sleep", return_value=None)
    @patch("builtins.print")  # Mock print
    def test_get_all_servo_serial_get_string_fails_twice(
        self, mock_print, mock_sleep, mock_get_string, mock_find
    ):
        mock_dev1 = MagicMock(iSerialNumber=1)
        mock_find.side_effect = [[mock_dev1], []]
        # Fail both times
        mock_get_string.side_effect = [
            ValueError("USB Error"),
            ValueError("USB Error Again"),
        ]

        result = get_all_servo_serial()

        assert result == []
        mock_sleep.assert_called_once_with(2)
        assert mock_get_string.call_count == 2
        mock_print.assert_called_once_with(
            "Failed to get serial number for device. Retry 1"
        )
