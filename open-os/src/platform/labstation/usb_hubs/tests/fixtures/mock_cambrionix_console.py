# Copyright 2024 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# pylint: disable=missing-module-docstring
# pylint: disable=missing-function-docstring
# pylint: disable=missing-class-docstring

import contextlib
import logging
import re

from cambrionix.console_lib import CONSOLE_CLEAR_CHR
from cambrionix.console_lib import CONSOLE_ENTER
import pytest


_logger = logging.getLogger("mock_cambrionix_host")


@pytest.fixture(scope="function")
def mock_cambrionix_console():
    def generate_cambrionix_console():
        class MockCambrionixConsole(contextlib.AbstractContextManager):
            def __init__(self):
                self.last_write = None
                self.written = b""
                self.console_responses = {
                    b"": None,
                }

                self.port_enabled_mapping = {
                    1: True,
                    2: True,
                    3: True,
                    4: True,
                    5: True,
                    6: True,
                    7: True,
                    8: True,
                    9: True,
                    10: True,
                    11: True,
                    12: True,
                    13: True,
                    14: True,
                    15: True,
                }

            def reset_input_buffer(self):
                pass

            def reset_output_buffer(self):
                pass

            def write(self, text):
                self.last_write = text.strip(
                    f"{CONSOLE_ENTER}{CONSOLE_CLEAR_CHR} ".encode("utf-8")
                )
                self.written += text
                if text.strip() == b"mode off":
                    self.port_enabled_mapping = {
                        port: False for port in self.port_enabled_mapping
                    }
                elif text.strip() == b"mode sync":
                    self.port_enabled_mapping = {
                        port: True for port in self.port_enabled_mapping
                    }
                match = re.match(r"mode off ([0-9]*)", text.strip().decode("utf-8"))
                if match:
                    self.port_enabled_mapping[int(match.group(1))] = False
                match = re.match(r"mode on ([0-9]*)", text.strip().decode("utf-8"))
                if match:
                    self.port_enabled_mapping[int(match.group(1))] = True

            def read(self):
                return None

            def read_until(self, unused_end_char):
                return self.console_responses[self.last_write]

            def flush(self):
                pass

            def __exit__(self, exc_type, exc_value, traceback):
                pass

        return MockCambrionixConsole()

    return generate_cambrionix_console
