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
from usb_hubs.tests.fixtures.mock_cambrionix_console import (
    mock_cambrionix_console,
)
from usb_hubs.tests.fixtures.mock_cambrionix_host import mock_cambrionix_host
from usb_hubs.tests.fixtures.mock_cambrionix_host_with_consoles import (
    mock_host_with_multiple_cambrionix,
)
from usb_hubs.tests.fixtures.mock_cambrionix_host_with_consoles import (
    mock_host_with_no_cambrionix,
)
from usb_hubs.tests.fixtures.mock_cambrionix_host_with_consoles import (
    mock_host_with_one_cambrionix,
)
