# Copyright 2023 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import sys
import unittest


"""Tests for /usr/bin/get_client_ui_scale.py."""

# Hackily import /usr/bin/get_desktop_scale as a module.
sys.path.insert(0, "/usr/bin")
import get_client_ui_scale  # pylint: disable=import-error,wrong-import-position


class TestGetClientUIScale(unittest.TestCase):
    """Tests normal calculation of the scale."""

    def test_parse_display_info(self):
        scale = get_client_ui_scale.get_scale(
            "Monitors: 2\n"
            "0: +*DP-4 2560/597x1440/336+0+0  DP-4\n"
            "1: +DP-6 2560/597x1440/336+2560+0  DP-6\n"
        )
        self.assertAlmostEqual(scale, 0.8378302)

    def test_malformed_input_raises_exception(self):
        """Checks that malformed input raises and exception."""
        with self.assertRaises(Exception):
            get_client_ui_scale.get_scale(
                "Monitors: 1\n" "0: +*DP-4 2560x597x1440x336+0+0  DP-4\n"
            )


if __name__ == "__main__":
    unittest.main()
