# Copyright 2024 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# These imports are necessary for pytest dependency injection.
# pylint: disable=unused-import
# pylint: disable=import-error
# pylint: disable=missing-module-docstring
# pylint: disable=missing-function-docstring
# pylint: disable=missing-class-docstring

import pytest
from image_downloader.tests.fixtures.mock_system import (
    mock_system,
)


from image_downloader.tests.fixtures.mock_system import (
    VALID_DRIVE,
    INVALID_DRIVE,
    VALID_LOCAL_BIN,
    INVALID_LOCAL_BIN,
    VALID_NETWORK_BIN,
    INVALID_NETWORK_BIN,
)
