# Copyright 2024 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Extract SVG representations of window positions/sizes from a window trace."""

import os.path
import random
import re
import sys
from typing import NamedTuple


LEGEND_WIDTH = 700
LEGEND_Y = 50
LEGEND_LINE_HEIGHT = 20


class Rect(NamedTuple):
    """Represents a window or monitor parsed from xwindump.py."""

    absolute_x: int
    absolute_y: int
    relative_x: int
    relative_y: int
    width: int
    height: int
    window_id: str
    name: str
    classes: str
    indent: int

    def caption(self) -> str:
        """Description of this Rect for display in the diagram."""
        return (
            "  " * self.indent
            + f"{self.window_id} {self.name}: ({self.classes})"
        )

    def legend(self) -> str:
        """Description of this Rect for display on the side of the diagram."""
        return (
            "  " * self.indent
            + f"{self.window_id} {self.name}: ({self.classes}) "
            f" {self.width}x{self.height}+{self.relative_x}+{self.relative_y} "
            f" +{self.absolute_x}+{self.absolute_y}"
        )

    def to_svg(
        self, legend_x: int, legend_y: int, caption_y_hint: float
    ) -> str:
        """Draw the window, its caption, and its legend text.

        Args:
            legend_x: X coordinate of the legend text.
            legend_y: Y coordinate of the legend text.
            caption_y_hint: A number between 0 and 1 indicating how far down the
                caption should be placed within the rectangle.

        Returns:
            A string containing a valid SVG fragment.
        """
        # Seed the RNG based on the window ID
        r = random.Random(sum([ord(c) for c in self.window_id]))
        red = hex(r.randint(0, 128))[2:].zfill(2)
        blue = hex(r.randint(0, 128))[2:].zfill(2)
        green = hex(r.randint(0, 128))[2:].zfill(2)
        colour = f"#{red}{green}{blue}"
        # Shift text around a little to reduce overlaps for coincident windows.
        text_y = self.absolute_y + self.height * caption_y_hint * 0.9 + 5
        return (
            f'<g class="window">  <rect x="{self.absolute_x}" y="{self.absolute_y}"'
            f' width="{self.width}" height="{self.height}" />\n  <text'
            f' x="{self.absolute_x + self.width * 0.4}" y="{text_y}"'
            f' class="caption"     style="--fill-color: {colour}">   '
            f' {self.caption()}   </text>\n  <text x="{legend_x}" y="{legend_y}"'
            f' class="legend"\n    style="--fill-color: {colour}">   '
            f" {self.legend()}</text>\n</g>"
        )


class Frame(NamedTuple):
    """Represents a single frame of the trace playback."""

    time: float
    log_message: str
    monitors: list
    windows: list

    def to_svg(self, frame_id: str) -> str:
        """Returns an SVG diagram of this frame."""
        max_width = 0
        max_height = 0
        for w in self.windows + self.monitors:
            if w.width > max_width:
                max_width = w.width
            if w.height > max_height:
                max_height = w.height
        max_height = max(
            max_height, LEGEND_Y + len(self.windows) * LEGEND_LINE_HEIGHT
        )
        total_width = max_width + LEGEND_WIDTH
        return (
            '<div class="frame">'
            f"  <p>[Time {self.time}] {self.log_message}</p>"
            f'  <svg id="{frame_id}"'
            f'   viewbox="0 0 {total_width} {max_height}" width="100%">\n'
            + "".join(
                r.to_svg(
                    max_width,
                    LEGEND_Y + i * LEGEND_LINE_HEIGHT,
                    i / len(self.windows),
                )
                for i, r in enumerate(self.monitors + self.windows)
            )
            + "  </svg>\n</div>\n"
        )


def parse_xwindump_line(line: str):
    """Extract rectangle coordinates from a single line of output.

    Matches monitors, like this:
    'XWAYLAND0 connected 1920x1080+0+0 (normal left inverted ...)'
    and also windows, like this:
    '0x3200001 "My Game": ("steam_app_123" "steam_app_123") 640x480+0+0  +10+20'

    Args:
        line: A line of output from xwininfo.

    Returns:
        A tuple of a Rect and a boolean (True for a monitor, False for a window).
    """
    is_monitor = line.startswith("XWAYLAND")
    if is_monitor:
        # "name" and "classes" don't really make sense for monitors so just
        # grab random parts of the string for now.
        match = re.match(
            r"(?P<id>XWAYLAND[0-9]*| *0x[0-9a-f]*) *(?P<name>.*)  *"
            r"(?P<width>[0-9]*)x(?P<height>[0-9]*)\+(?P<x>[0-9]*)\+(?P<y>[0-9]*) "
            r" *\((?P<classes>.*)\)",
            line,
        )
    else:
        match = re.match(
            r" *(?P<id>0x[0-9a-f]*) (?P<name>.*): \((?P<classes>.*)\)  *"
            r"(?P<width>[0-9]*)x(?P<height>[0-9]*)\+(?P<x>[0-9]*)\+(?P<y>[0-9]*)"
            r"  *\+(?P<abs_x>[0-9]*)\+(?P<abs_y>[0-9]*)",
            line,
        )
    if not match:
        return None, is_monitor
    window_id = match.group("id")
    name = match.group("name")
    classes = match.group("classes")
    w = int(match.group("width"))
    h = int(match.group("height"))
    rel_x = int(match.group("x"))
    rel_y = int(match.group("y"))
    abs_x = rel_x if is_monitor else int(match.group("abs_x"))
    abs_y = rel_y if is_monitor else int(match.group("abs_y"))
    indent = (len(line) - len(line.lstrip())) // 2

    return (
        Rect(
            abs_x, abs_y, rel_x, rel_y, w, h, window_id, name, classes, indent
        ),
        is_monitor,
    )


def xwindump_to_frame(xwindump: str, time: float, log_message: str) -> Frame:
    """Given the output of xwindump.py, return a Frame.

    Args:
        xwindump: Raw output from xwindump.py.
        time: Time of the new frame.
        log_message: A message to display alongside the frame.

    Returns:
        A Frame.

    For example, for the following input (line breaks added for readability):

    '''
      XWAYLAND0 connected 1920x1080+0+0 (normal left inverted right x axis y axis)
      310mm x 174mm

        0x20000c (has no name): ()  640x480+0+0  +0+0
          1 child:
          0x3200001 "Title": ("steam_app_322170" "steam_app_322170")
          640x480+0+0  +0+0
             1 child:
             0x3000028 (has no name): ()  320x240+0+0  +0+0
      '''

     The output would be:
     (
       Frame(
         0,
         'Initial state',
         [window(0, 0, 1920, 1080)]
         [window(0, 0, 640, 480), window(0, 0, 320, 240)],
       ),
     )
    """
    monitors = []  # type: list[Rect]
    windows = []  # type: list[Rect]
    for line in xwindump.split("\n"):
        if line.startswith("=== Window "):
            # We've passed the Monitors and Root window sections, stop now.
            break

        window, is_monitor = parse_xwindump_line(line)
        if window:
            if is_monitor:
                monitors.append(window)
            else:
                windows.append(window)
    return Frame(time, log_message, monitors, windows)


def fill_html_template(title: str, frames: list) -> str:
    """Fills a predefined HTML template with the given content.

    Args:
        title: The title of the HTML document.
        frames: Frames to render within the document as SVG.

    Returns:
        The contents of rects_template.html, with %TITLE% replaced by the title
        parameter and %FRAMES% replaced by the frames parameter.
    """
    frames_svg = "".join(
        frame.to_svg(f"frame{i}") for i, frame in enumerate(frames)
    )
    with open(
        os.path.join(
            os.path.dirname(sys.argv[0]),
            "windowtrace",
            "visualizer_template.html",
        ),
        encoding="utf-8",
    ) as f:
        return (
            f.read().replace("%TITLE%", title).replace("%FRAMES%", frames_svg)
        )
