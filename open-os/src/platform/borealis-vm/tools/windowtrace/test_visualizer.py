# Copyright 2024 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Tests for visualizer.py."""

import unittest

from . import visualizer


class TestSvgGeneration(unittest.TestCase):
    """Test creating SVGs from Rects and Frames."""

    def test_rect_to_svg(self):
        """Test the visualizer.Rect.to_svg() method."""
        window = visualizer.Rect(
            10,
            20,
            0,
            0,
            30,
            40,
            "0x3200001",
            "My Game",
            "",
            2,
        )
        result = window.to_svg(100, 200, 0)
        self.assertIn('<rect x="10" y="20" width="30" height="40"', result)
        self.assertIn('<text x="100" y="200" class="legend"', result)
        self.assertIn("0x3200001 My Game: ()  30x40+0+0  +10+20", result)

    def test_frame_to_svg(self):
        """Test the visualizer.Frame.to_svg() method."""
        frame = visualizer.Frame(
            time=0,
            log_message="",
            monitors=[
                visualizer.Rect(0, 0, 0, 0, 1920, 1080, "XWAYLAND0", "", "", 0)
            ],
            windows=[
                visualizer.Rect(
                    10,
                    20,
                    10,
                    20,
                    30,
                    40,
                    "0x3200001",
                    "My Game",
                    "My Game 30x40+10+20",
                    2,
                ),
                visualizer.Rect(
                    0,
                    0,
                    0,
                    0,
                    640,
                    480,
                    "0x3200002",
                    "Title",
                    "Title 640x480+0+0",
                    2,
                ),
                visualizer.Rect(
                    0,
                    0,
                    0,
                    0,
                    320,
                    240,
                    "0x3200003",
                    "Title",
                    "Title 320x240+0+0",
                    4,
                ),
            ],
        )
        result = frame.to_svg("test")
        self.assertIn('<rect x="0" y="0" width="1920" height="1080"', result)
        self.assertIn('<rect x="10" y="20" width="30" height="40"', result)
        self.assertIn('<rect x="0" y="0" width="640" height="480"', result)
        self.assertIn('<rect x="0" y="0" width="320" height="240"', result)

    def test_xwindump_to_frame(self):
        """Test parsing xwindump.py output."""
        xwindump = """XWAYLAND0 connected 1920x1080+0+0 (...) 310mm x 174mm

      0x20000c (has no name): ()  640x480+0+0  +0+0
        1 child:
        0x3200002 "Default IME": ("app_123" "app_123")  640x480+0+0  +0+0
           1 child:
           0x3000028 (has no name): ()  320x240+0+0  +0+0
    """
        frame = visualizer.xwindump_to_frame(xwindump, 0, "Hello world")
        self.assertEqual([r.width for r in frame.monitors], [1920])
        self.assertEqual([r.width for r in frame.windows], [640, 640, 320])


TEST_WINDOWS = [
    visualizer.Rect(
        absolute_x=10,
        absolute_y=20,
        relative_x=10,
        relative_y=20,
        width=30,
        height=40,
        window_id="0x3200001",
        name="",
        classes="",
        indent=1,
    ),
    visualizer.Rect(
        absolute_x=1000,
        absolute_y=2000,
        relative_x=1000,
        relative_y=2000,
        width=640,
        height=480,
        window_id="0x3200002",
        name="",
        classes="",
        indent=1,
    ),
    visualizer.Rect(
        absolute_x=1050,
        absolute_y=2060,
        relative_x=50,
        relative_y=60,
        width=320,
        height=240,
        window_id="0x3200003",
        name="",
        classes="",
        indent=2,
    ),
]


class TestParseXwindump(unittest.TestCase):
    """Test parsing of xwindump.py output."""

    def test_monitor(self):
        """Test parsing a line of xrandr's output, representing a monitor."""
        line = (
            "XWAYLAND0 connected 1920x1080+0+0 (normal left inverted right x axis y"
            " axis)"
        )
        window, is_monitor = visualizer.parse_xwindump_line(line)
        self.assertTrue(is_monitor)
        self.assertEqual(window.absolute_x, 0)
        self.assertEqual(window.absolute_y, 0)
        self.assertEqual(window.relative_x, 0)
        self.assertEqual(window.relative_y, 0)
        self.assertEqual(window.width, 1920)
        self.assertEqual(window.height, 1080)
        self.assertEqual(window.window_id, "XWAYLAND0")
        self.assertEqual(
            window.caption(),
            "XWAYLAND0 connected: (normal left inverted right x axis y axis)",
        )
        self.assertIn("XWAYLAND0 connected", window.legend())
        self.assertIn("1920x1080+0+0", window.legend())

    def test_window(self):
        """Test parsing a line of xwininfo -tree, representing a window."""
        line = (
            '0x3200001 "My Game": ("steam_app_123" "steam_app_123")  640x480+0+0 '
            " +10+20"
        )
        window, is_monitor = visualizer.parse_xwindump_line(line)
        self.assertFalse(is_monitor)
        self.assertEqual(window.absolute_x, 10)
        self.assertEqual(window.absolute_y, 20)
        self.assertEqual(window.relative_x, 0)
        self.assertEqual(window.relative_y, 0)
        self.assertEqual(window.width, 640)
        self.assertEqual(window.height, 480)
        self.assertEqual(window.window_id, "0x3200001")
        self.assertEqual(
            window.caption(),
            '0x3200001 "My Game": ("steam_app_123" "steam_app_123")',
        )
        self.assertEqual(window.legend(), line)
