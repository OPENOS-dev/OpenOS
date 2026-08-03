# Copyright 2024 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# pylint: disable=missing-module-docstring
# pylint: disable=missing-function-docstring
# pylint: disable=missing-class-docstring
# pylint: disable=import-error
# pylint: disable=no-name-in-module

import logging

import pytest


_logger = logging.getLogger("mock_cambrionix_host_with_console")


@pytest.fixture()
def mock_host_with_no_cambrionix(mock_cambrionix_host):
    def generate_host():
        mock_cambrionix_host()

    return generate_host


@pytest.fixture()
def mock_host_with_one_cambrionix(mock_cambrionix_host, mock_cambrionix_console):
    def generate_host():
        cambrionix_console = mock_cambrionix_console()
        cambrionix_host = mock_cambrionix_host()
        cambrionix_host.set_usb_devices(
            {"usb-cambrionix_SuperSync15_0000003B71550C92-if01": cambrionix_console}
        )

        return cambrionix_host

    return generate_host


@pytest.fixture()
def mock_host_with_multiple_cambrionix(mock_cambrionix_host, mock_cambrionix_console):
    def generate_host():

        cambrionix_console = mock_cambrionix_console()
        cambrionix_host = mock_cambrionix_host()
        cambrionix_host.set_usb_devices(
            {
                "usb-cambrionix_SuperSync15_0000003B71550C92-if01": cambrionix_console,
                "usb-cambrionix_SuperSync15_0000003B71550C93-if01": cambrionix_console,
            }
        )

        return cambrionix_host

    return generate_host
