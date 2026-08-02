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

from image_downloader.tests.fixtures.mock_system import (
    VALID_DRIVE,
    INVALID_DRIVE,
    VALID_LOCAL_BIN,
    INVALID_LOCAL_BIN,
    VALID_NETWORK_BIN,
    INVALID_NETWORK_BIN,
)


class TestWorkingTransfers:
    def test_working_local(self, monkeypatch, mock_system):

        mock_system()
        main(["-d", VALID_DRIVE, "-i", VALID_LOCAL_BIN])

    def test_working_network(self, monkeypatch, mock_system):

        mock_system()
        main(["-d", VALID_DRIVE, "-i", VALID_NETWORK_BIN])


class TestFailedTransfers:
    def test_no_drive(self, monkeypatch, mock_system):

        mock_system()

        with pytest.raises(ImageDownloaderException) as pytest_wrapped_e:
            main(["-d", INVALID_DRIVE, "-i", VALID_NETWORK_BIN])

    def test_no_local_bin(self, monkeypatch, mock_system):

        mock_system()

        with pytest.raises(ImageDownloaderException) as pytest_wrapped_e:
            main(["-d", VALID_DRIVE, "-i", INVALID_LOCAL_BIN])

    def test_no_network_bin(self, monkeypatch, mock_system):

        mock_system()

        with pytest.raises(ImageDownloaderException) as pytest_wrapped_e:
            main(["-d", VALID_DRIVE, "-i", INVALID_NETWORK_BIN])
