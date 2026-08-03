# Copyright 2024 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Tests for window_groups.py."""

import unittest

from . import window_groups


class FakeLogLine:
    """Fake version of the LogLine class from the main program."""

    def __init__(self, text, source="sommelier → Exo"):
        self.text = text
        self.source = source
        self.window = ""


class TestWindowGrouping(unittest.TestCase):
    """Test that window objects are correctly grouped."""

    def setUp(self):
        self.grouper = window_groups.WindowGrouper()

    def process(self, logs):
        """Exercise the WindowGrouper with the given logs."""
        for l in logs:
            self.grouper.parse(l)
        self.grouper.populate_window_columns()

    def test_one_group(self):
        """Test that several objects can be grouped together."""
        logs = [
            FakeLogLine("→ wl_surface@94.attach(wl_buffer@95, 0, 0)"),
            FakeLogLine(
                "→ wp_viewporter@8.get_viewport(new id wp_viewport@92, wl_surface@94)"
            ),
            FakeLogLine(
                "→ xdg_wm_base@15.get_xdg_surface(new id xdg_surface@71, wl_surface@94)"
            ),
            FakeLogLine(
                "→ xdg_surface@71.get_toplevel(new id xdg_toplevel@66)"
            ),
            FakeLogLine(
                "→ zaura_shell@11.get_aura_surface(new id zaura_surface@74, wl_surface@94)"
            ),
            FakeLogLine("→ wl_surface@94.frame(new id wl_callback@86)"),
        ]
        self.process(logs)
        for l in logs:
            self.assertEqual(
                l.window,
                "zaura_surface@74 [host], "
                "xdg_toplevel@66 [host], "
                "xdg_surface@71 [host], "
                "wp_viewport@92 [host], "
                "wl_surface@94 [host]",
            )

    def test_two_groups(self):
        """Test that unrelated objects are correctly kept separate."""
        logs = [
            FakeLogLine("→ xdg_surface@5.get_toplevel(new id xdg_toplevel@6)"),
            FakeLogLine(
                "→ xdg_surface@100.get_toplevel(new id xdg_toplevel@200)"
            ),
        ]
        self.process(logs)
        self.assertEqual(
            [l.window for l in logs],
            [
                "xdg_toplevel@6 [host], xdg_surface@5 [host]",
                "xdg_toplevel@200 [host], xdg_surface@100 [host]",
            ],
        )

    def test_assigning_title(self):
        """Test that xdg_toplevel titles are also printed."""
        logs = [
            FakeLogLine(
                "→ xdg_wm_base@15.get_xdg_surface(new id xdg_surface@71, wl_surface@94)"
            ),
            FakeLogLine(
                "→ xdg_surface@71.get_toplevel(new id xdg_toplevel@66)"
            ),
            FakeLogLine('→ xdg_toplevel@66.set_title("My First Game")'),
        ]
        self.process(logs)
        self.assertEqual(
            logs[0].window,
            "My First Game: xdg_toplevel@66 [host], xdg_surface@71 [host], wl_surface@94 [host]",
        )

    def test_separate_by_source(self):
        """Test that Wayland objects from different connections are separately namespaced."""
        logs = [
            FakeLogLine("→ wl_surface@300.frame(new id wl_callback@1)", "Exo"),
            FakeLogLine(
                "→ wl_surface@300.frame(new id wl_callback@2)", "XWayland"
            ),
        ]
        self.process(logs)
        self.assertEqual(
            [l.window for l in logs],
            ["wl_surface@300 [host]", "wl_surface@300"],
        )

    def test_object_deletion(self):
        """Test that objects with reused IDs do not share an identity."""
        logs = [
            # Create and attach a new buffer
            FakeLogLine(
                " → wl_drm@6.create_prime_buffer("
                + "new id wl_buffer@53, fd 45, 1024, 768, 875713112, 0, 4096, 0, 0, 0, 0)",
                "Xwayland",
            ),
            FakeLogLine(
                " → wl_surface@1.attach(wl_buffer@53, 0, 0)", "Xwayland"
            ),
            # Create a new buffer with the same name and attach to a different surface
            FakeLogLine(
                " → wl_drm@6.create_prime_buffer("
                + "new id wl_buffer@53, fd 45, 1024, 768, 875713112, 0, 4096, 0, 0, 0, 0)",
                "Xwayland",
            ),
            FakeLogLine(
                " → wl_surface@2.attach(wl_buffer@53, 0, 0)", "Xwayland"
            ),
        ]
        self.process(logs)
        self.assertEqual(
            [l.window for l in logs],
            # Surfaces are not linked together.
            ["wl_surface@1", "wl_surface@1", "wl_surface@2", "wl_surface@2"],
        )

    def test_recreated_objects_have_generation(self):
        """Test that objects with reused IDs have letters appended to distinguish them."""
        logs = [
            FakeLogLine(
                "→ wl_compositor@4.create_surface(new id wl_surface@100)",
                "Xwayland → sommelier",
            ),
            FakeLogLine(
                "→ wl_compositor@4.create_surface(new id wl_surface@100)",
                "Xwayland → sommelier",
            ),
        ]
        self.process(logs)
        self.assertEqual(
            [l.window for l in logs],
            ["wl_surface@100", "wl_surface@100b"],
        )

    def test_grouping_across_guest_boundary(self):
        """Test that window groups are formed across the host/guest boundary."""
        logs = [
            FakeLogLine(
                "wl_surface@52.attach(wl_buffer@51, 0, 0)",
                "sommelier → XWayland",
            ),
            FakeLogLine(
                "→ wl_surface@94.attach(wl_buffer@99, 0, 0)",
                "sommelier → Exo",
            ),
        ]
        self.process(logs)
        window_group = "wl_surface@94 [host], wl_surface@52"
        self.assertEqual([l.window for l in logs], [window_group, window_group])

    def test_grouping_across_guest_boundary_different_operation(self):
        """Test that window groups are not formed if operations don't match."""
        logs = [
            FakeLogLine(
                "wl_surface@52.attach(wl_buffer@51, 0, 0)",
                "sommelier → XWayland",
            ),
            FakeLogLine(
                "→ wl_surface@94.damage(0, 0, 1, 1)",
                "sommelier → Exo",
            ),
        ]
        self.process(logs)
        self.assertEqual(
            [l.window for l in logs], ["wl_surface@52", "wl_surface@94 [host]"]
        )

    def test_grouping_x11_and_wayland(self):
        """Wayland and X11 get grouped."""
        logs = [
            FakeLogLine(
                "← 33 (ClientMessage) { "
                "format: 32; "
                "event-sequencenumber: 1257; "
                "eventwindow: 0x00200011; "
                "type: 254 (WL_SURFACE_ID); "
                "data: 3400000000000000000000000000000000000000 }",
                "sommelier | PID 388",
            )
        ]
        self.process(logs)
        self.assertEqual(logs[0].window, "wl_surface@52, 0x00200011")

    def test_grouping_reparented_windows(self):
        """Windows are grouped with their frame windows when reparented."""
        logs = [
            FakeLogLine(
                "→ 7 (ReparentWindow) { window: 0x03800001; parent: 0x00200011; x: 0; y: 0 }",
                "sommelier | PID 388",
            )
        ]
        self.process(logs)
        self.assertEqual(logs[0].window, "0x03800001, 0x00200011")

    def test_x11_window_titles(self):
        """X11 window titles are parsed."""
        logs = [
            FakeLogLine(
                "→ 8 (MapWindow) { window: 0x03800001 (My Game) }",
                "wineserver | PID 123, MyGame.exe | PID 1024",
            )
        ]
        self.process(logs)
        self.assertEqual(logs[0].window, "My Game: 0x03800001")

    def test_not_all_window_ids_are_grouped(self):
        """Don't always group window IDs; in this example, don't group with the root window."""
        logs = [
            FakeLogLine(
                "→ 1 (CreateWindow) { depth: 24; wid: 0x03800002; parent: 0x000005aa; "
                "x: 0; y: 0; width: 1; height: 1 } ",
                "wineserver | PID 1441, game.exe | PID 1802",
            )
        ]
        self.process(logs)
        self.assertEqual(logs[0].window, "0x03800002")

    def test_drawables_are_not_always_windows(self):
        """Drawables shouldn't create window groups on their own."""
        logs = [
            FakeLogLine(
                "→ 55 (CreateGC) { "
                "cid: 0x04a00091; drawable: 0x03800001; gc-value-mask: 0x00000000 }",
                "game.exe | PID 1802",
            )
        ]
        self.process(logs)
        self.assertFalse(logs[0].window)

    def test_windows_are_drawables(self):
        """Drawables which *are* windows should be tagged with window groups."""
        logs = [
            FakeLogLine(
                "→ 1 (CreateWindow) { wid: 0x04a000a4 }",
                "game.exe | PID 1802",
            ),
            FakeLogLine(
                "→ 15 (QueryTree) { drawable: 0x04a000a4 }",
                "game.exe | PID 1802",
            ),
        ]
        self.process(logs)
        self.assertEqual([l.window for l in logs], ["0x04a000a4", "0x04a000a4"])
