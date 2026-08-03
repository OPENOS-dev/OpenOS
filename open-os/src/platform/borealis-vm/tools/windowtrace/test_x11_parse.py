# Copyright 2024 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Tests for x11_parse.py."""

import unittest

from . import x11_parse


SAMPLE_CLIENTMESSAGE = (
    " ← 33 (ClientMessage) { "
    "format: 32; "
    "event-sequencenumber: 1257; "
    "eventwindow: 0x00200011; "
    "type: 254 (WL_SURFACE_ID); "
    "data: 3400000000000000000000000000000000000000 }"
)

# Message changing a window title to the string '; format: 9;' in an attempt to defeat the parser.
SAMPLE_EMBEDDED_CHARACTERS = (
    "→ 18 (ChangeProperty) { "
    "mode: 0 (Replace); "
    r"window: 0x03800002 (; format: 9;); "
    "property: 262 (_NET_WM_NAME); "
    "type: 255 (UTF8_STRING); "
    "format: 8; "
    r'data: 666f726d61743a2039 "; format: 9;" } '
)

SAMPLE_WITH_WINDOW_TITLE = (
    "← 28 (PropertyNotify) { "
    "event-sequencenumber: 112; "
    "eventwindow: 0x03800001 (My Game); "
    "atom: 249 (WM_STATE); "
    "time: 349812; "
    "property-state: 0 (NewValue) }"
)


class TestX11Parse(unittest.TestCase):
    """Test we can parse X11 messages as produced by Wireshark."""

    def test_atom_parameter(self):
        """Test names are extracted from atom parameters (the atom number is dropped)."""
        p = x11_parse.parameters(SAMPLE_CLIENTMESSAGE)
        self.assertEqual(p["type"], ["WL_SURFACE_ID"])

    def test_id_parameter(self):
        """Test that a simple window ID parameter is parsed."""
        p = x11_parse.parameters(SAMPLE_CLIENTMESSAGE)
        self.assertEqual(p["eventwindow"], ["0x00200011"])

    def test_data_parameter(self):
        """Test that a data parameter is successfully decoded."""
        p = x11_parse.parameters(SAMPLE_CLIENTMESSAGE)
        self.assertEqual(x11_parse.decode_hex_data_as_int(p["data"][0]), 0x34)

    def test_embedded_characters(self):
        """Test key/value delimiters inside parentheses/strings aren't treated as delimiters."""
        p = x11_parse.parameters(SAMPLE_EMBEDDED_CHARACTERS)
        self.assertEqual(p["format"], ["8"])

    def test_window_titles(self):
        """Test that window parameters with titles are parsed correctly."""
        p = x11_parse.parameters(SAMPLE_WITH_WINDOW_TITLE)
        self.assertEqual(p["eventwindow"], ["0x03800001 (My Game)"])

    def test_window_param(self):
        """Test that window IDs can be extracted."""
        p = x11_parse.parameters(SAMPLE_CLIENTMESSAGE)
        self.assertEqual(x11_parse.window_param(p, "eventwindow"), "0x00200011")

    def test_window_param_with_title(self):
        """Test that window IDs can be extracted if there's a window title."""
        p = x11_parse.parameters(SAMPLE_WITH_WINDOW_TITLE)
        self.assertEqual(x11_parse.window_param(p, "eventwindow"), "0x03800001")

    def test_title_from_window_param(self):
        """Test that window titles can be extracted if present."""
        p = x11_parse.parameters(SAMPLE_WITH_WINDOW_TITLE)
        self.assertEqual(
            x11_parse.title_from_window_param(p, "eventwindow"), "My Game"
        )
