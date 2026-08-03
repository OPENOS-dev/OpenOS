# Copyright 2022 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Tests for window_trace.py"""

import tempfile
import unittest

from . import window_trace


class TestWindowTrace(unittest.TestCase):
    """Tests for window_trace.py"""

    def test_file_ends_with(self):
        """Test window_trace.file_ends_with()"""
        with tempfile.TemporaryFile() as f:
            f.write(b"one\ntwo\nthree\n")
            self.assertTrue(window_trace.file_ends_with(f, b"three\n"))
            self.assertFalse(window_trace.file_ends_with(f, b"three"))
            self.assertFalse(window_trace.file_ends_with(f, b"two"))
            f.write(b"four\n")
            self.assertFalse(window_trace.file_ends_with(f, b"three\n"))
            self.assertTrue(window_trace.file_ends_with(f, b"four\n"))
