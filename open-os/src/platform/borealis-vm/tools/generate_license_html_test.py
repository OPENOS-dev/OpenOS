#!/usr/bin/env python3
# Copyright 2023 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Tests for generate_license_html.py."""

import unittest

import generate_license_html


class TestParsing(unittest.TestCase):
    """Test parsing license strings from pacman -Qi."""

    def test_split_licenses(self):
        """Test trivial license string splitting."""
        parsed = generate_license_html.split_licenses(
            "BSD  custom:foo  hello world  test"
        )
        self.assertEqual(
            set(parsed),
            {"BSD", "custom:foo", "hello world", "test"},
        )

    def test_simple_spdx_expression(self):
        """Test that we can handle a simple SPDX expression."""
        parsed = generate_license_html.split_licenses(
            "GPL-2.0-or-later OR LGPL-3.0-or-later"
        )
        self.assertEqual(
            set(parsed),
            {"GPL-2.0-or-later", "LGPL-3.0-or-later"},
        )

    def test_complicated_spdx_expression(self):
        """Test that we can handle a bigger SPDX expression."""
        parsed = generate_license_html.split_licenses(
            "(RandomLicense AND Foo) OR (((GPL-2.0-or-later))) OR (LGPL-3.0-or-later AND BSD)"
        )
        self.assertEqual(
            set(parsed),
            {
                "RandomLicense",
                "Foo",
                "GPL-2.0-or-later",
                "LGPL-3.0-or-later",
                "BSD",
            },
        )


if __name__ == "__main__":
    unittest.main()
