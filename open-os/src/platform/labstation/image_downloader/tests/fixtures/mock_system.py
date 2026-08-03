# Copyright 2024 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import sys
import os
import subprocess

import requests

import pytest
import unittest.mock

from image_downloader.image_downloader import main
from image_downloader.image_downloader import ImageDownloader
from image_downloader.image_downloader import ImageDownloaderException


import pytest


VALID_DRIVE = "/dev/valid"
INVALID_DRIVE = "/dev/invalid"

VALID_LOCAL_BIN = "valid.bin"
INVALID_LOCAL_BIN = "invalid.bin"

VALID_NETWORK_BIN = "https://google.com/valid.bin"
INVALID_NETWORK_BIN = "https://google.com/invalid.bin"


@pytest.fixture()
def mock_system(class_mocker, mocker):
    def generate_mocker():
        """Mock generator function.   This allows multiple tests to be run in
        parallel as it generates a new mock for each test vs sharing the same
        mock between tests.
        """

        class SystemMocker:
            def __init__(self):
                self.usb_dev = VALID_DRIVE
                self.local_path = VALID_LOCAL_BIN
                self.network_path = VALID_NETWORK_BIN
                self.bin_size = 10000

                class_mocker.patch(
                    "subprocess.run", return_value=subprocess.CompletedProcess("", 0)
                )

                class_mocker.patch("os.path.exists", side_effect=self.mock_path_exists)

                class_mocker.patch(
                    "os.statvfs", return_value=os.statvfs_result([0] * 10)
                )

                class_mocker.patch("builtins.open", unittest.mock.mock_open())

                class_mocker.patch("shutil.copyfile", side_effect=self.mock_copyfile)

                class_mocker.patch("requests.head", side_effect=self.mock_requests_head)

                class_mocker.patch("requests.get", side_effect=self.mock_requests_get)

            def mock_path_exists(self, path):
                return self.usb_dev == path

            def mock_copyfile(self, src, dst):
                if self.local_path != src or self.usb_dev != dst:
                    raise FileNotFoundError()

            def mock_requests_head(self, url, **kwargs):
                resp = requests.Response()
                resp.status_code = 200
                if self.network_path != url:
                    resp.status_code = 404
                resp.close = print
                return resp

            def mock_requests_get(self, url, **kwargs):
                resp = requests.Response()
                resp.status_code = 200
                resp.headers["content-length"] = str(self.bin_size)
                if self.network_path != url:
                    resp.status_code = 404
                resp.iter_content = (
                    lambda chunk_size=1, decode_unicode=False: "0" * self.bin_size
                )
                resp.close = lambda: True
                return resp

        return SystemMocker()

    return generate_mocker
