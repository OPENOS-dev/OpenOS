# Copyright 2023 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Tests for log categorization."""

import unittest

from . import categories


class TestParseX11Parameter(unittest.TestCase):
    """Tests for parsing X11 parameters."""

    def test_extension_minor(self):
        """Test that the extension-minor parameter's value is parsed."""
        self.assertEqual(
            "Pixmap",
            categories.x11_parameter(
                "→ 146 (Present) { "
                "extension-minor: 1 (Pixmap); "
                "window: 0x04c00121; "
                "pixmap: 0x04c00129 }",
                "extension-minor",
            ),
        )


class TestParseX11Operation(unittest.TestCase):
    """Tests for parsing X11 operations for categorization."""

    def test_extension_minor(self):
        """Test extension-minor parameter is included in operation names."""
        self.assertEqual(
            "Present Pixmap",
            categories.x11_operation(
                "→ 146 (Present) { "
                "extension-minor: 1 (Pixmap); "
                "window: 0x04c00121; "
                "pixmap: 0x04c00129 }"
            ),
        )

    def test_property(self):
        """Test property parameter is included in operation names."""
        self.assertEqual(
            "GetProperty RESOURCE_MANAGER",
            categories.x11_operation(
                "→ 20 (GetProperty) { "
                "delete: False; "
                "window: 0x000005aa; "
                "property: 23 (RESOURCE_MANAGER); "
                "get-property-type: 31 (STRING) }"
            ),
        )


class TestCategory(unittest.TestCase):
    """Test category assignment."""

    def test_category_propagates_to_deletions(self):
        """Test that delete_id events have the same categories as their matching creation event."""
        c = categories.Categorizer()
        cats, _ = c.categories(
            "→ wl_compositor@4.create_surface(new id wl_surface@64)"
        )
        self.assertEqual(
            cats,
            c.categories("wl_display@1.delete_id(64)")[0],
        )


if __name__ == "__main__":
    unittest.main()
