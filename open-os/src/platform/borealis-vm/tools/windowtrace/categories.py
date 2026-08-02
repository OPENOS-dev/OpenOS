# Copyright 2023 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Categorise lines of the trace, to enable filtering the spreadsheet."""

import collections
from enum import StrEnum
from enum import UNIQUE
from enum import verify
import itertools
import re


@verify(UNIQUE)
class Category(StrEnum):
    """Categories assigned to each line of the logs in the trace."""

    # Connection setup, X11 extensions/atoms, Wayland registry, internal errors.
    INTERNAL = "Internals"
    # X11 drawing operations: Graphics Contents (GCs), clip masks, etc.
    X11_DRAWING = "X11 drawing operations"
    # Keyboard (X11 KeyPress/KeyRelease, wl_keyboard), mouse/gamepad buttons
    INPUT = "Input: keys/buttons"
    # Mouse movements, gamepad axis changes. Broken out as they can be spammy.
    INPUT_MOTION = "Input: movements"
    # Usage of the clipboard, drag operations, etc.
    CLIPBOARD = "Clipboard"
    # Anything related to presenting rendered frames for display.
    PRESENTATION = "Presentation"
    # Updates to output state (wl_output, RANDR)
    OUTPUTS = "Outputs (monitors)"
    # wl_display::sync, wl_callback
    SYNC = "Sync"
    # Window creation, mapping, destruction
    WINDOW_LIFECYCLE = "Window lifecycle"
    # Window positioning and sizing
    WINDOW_RECT = "Window position/size"
    # Whether a window has a frame, and what those frame decorations look like.
    WINDOW_FRAME = "Window frame"
    # Fullscreen, maximized, minimized.
    WINDOW_STATE = "Window state"
    # Other X11 properties not captured elsewhere.
    OTHER_X11_PROPERTY = "Other X11 property"
    # Window focus/expose
    FOCUS = "Focus"
    # Typically uninteresting operations.
    MISC = "Misc"
    # Unparsed or nonsensical operations.
    UNKNOWN = "Unknown"


def x11_property_operations(props):
    """Return a list of X11 operations involving the given properties."""
    return [
        f"{o} {p}"
        for (o, p) in itertools.product(
            [
                "GetProperty",
                "ChangeProperty",
                "PropertyNotify",
                "DeleteProperty",
            ],
            props,
        )
    ]


Match = collections.namedtuple("Match", ["log_regexes", "operations"])

# Category assignment rules.
#
# For ease of maintenance (and execution speed), there are two supported ways to assign a category
# to a line of the windowing trace (equivalently, a row of the spreadsheet):
#
# log_regexes: Regexes to run on the entire log line. Fully flexible, but more complex and slower.
#
# operations: Exact matches to the return value of x11_operation() or wayland_operation(),
#     or to the first word of those return values.
#     These are short summaries of the type of request/reply/error represented in the log.
#     Generally it's just the request/reply/error name, but in some cases a relevant argument is
#     extracted to provide more context. For example, ChangeProperty requests have the property
#     name appended. The categorizer will first match against the entire operation
#     (like "ChangeProperty _NET_WM_STATE"), and fall back to matching only the first word
#     (like "ChangeProperty") only if no other category could be assigned.
#
# Operations are also shown in the spreadsheet's Operation column, to allow more granular filtering.
#
# TODO(cpelling): Categorize more logs.
category_matchers = {
    Category.INTERNAL: Match(
        log_regexes=[
            r"^→ Initial connection request",
            r"^← Initial connection reply",
        ],
        operations=[
            "QueryExtension",
            "BIG_REQUESTS",
            "BIG-REQUESTS",
            "Generic Event Extension",
            "Generic Event Extension QueryVersion",
            "XInputExtension",
            "XInputExtension GetExtensionVersion",
            "XInputExtension XIQueryVersion",
            "XFIXES",
            "XFIXES QueryVersion",
            "InternAtom",
            "GetAtomName",
            "wl_registry.bind",
            "wl_registry.global",
            "wl_registry.global_remove",
        ]
        + x11_property_operations(["WM_LOCALE_NAME", "RESOURCE_MANAGER"]),
    ),
    Category.X11_DRAWING: Match(
        log_regexes=[],
        operations=[
            "CreateGC",
            "ChangeGC",
            "FreeGC",
            "SetClipRectangles",
            "CreatePixmap",
            "PutImage",
            "CreateColormap",
            "BadDrawable",
            "BadGC",
            "BadPixmap",
            "FreePixmap",
            "FreeColormap",
            "Sent-ColormapNotify",
        ]
        + x11_property_operations(["WM_COLORMAP_WINDOWS"]),
    ),
    Category.INPUT: Match(
        log_regexes=[],
        operations=[
            "ButtonPress",
            "ButtonRelease",
            "KeyPress",
            "KeyRelease",
            "KeymapNotify",
            "XKEYBOARD",
            "wl_keyboard",
            "wl_touch.down",
            "wl_touch.up",
            "wl_touch.cancel",
            "XInputExtension XIGetClientPointer",
        ],
    ),
    Category.INPUT_MOTION: Match(
        log_regexes=[],
        operations=[
            "MotionNotify",
            "QueryPointer",
            "wl_pointer",
            "wl_touch",
            "zwp_relative_pointer_v1.relative_motion",
        ],
    ),
    Category.UNKNOWN: Match(
        log_regexes=[
            r"^Reply to unknown request",
        ],
        operations=[
            "Success",  # X11 error code 0
            "<Unknown",  # <Unknown eventcode...>, <Unknown opcode...>
            "Sent-<Unknown",  # <Sent-Unknown eventcode...>
        ],
    ),
    # Buffer attach/commit/release and X11 equivalents; render properties.
    Category.PRESENTATION: Match(
        log_regexes=[],
        operations=[
            "Present",
            "Present Pixmap",
            "Present QueryCapabilities",
            "Present QueryVersion",
            "Present SelectInput",
            "GenericEvent Present",
            "wl_surface.attach",
            "wl_surface.enter",
            "wl_surface.leave",
            "wl_surface.damage",
            "wl_surface.damage_buffer",
            "wl_surface.frame",
            "wl_surface.commit",
            "wl_drm.create_prime_buffer",
            "wl_shm_pool.create_buffer",
            "zwp_linux_dmabuf_v1.create_params",
            "zwp_linux_buffer_params_v1.add",
            "zwp_linux_buffer_params_v1.create_immed",
            "zwp_linux_buffer_params_v1.destroy",
            "wl_buffer.release",
            "wl_buffer.destroy",
            "wl_pointer.set_cursor",
            "DRI3",
            "DRI3 FenceFromFD",
            "DRI3 GetSupportedModifiers",
            "DRI3 Open",
            "DRI3 PixmapFromBuffer",
            "DRI3 QueryVersion",
        ]
        + x11_property_operations(
            [
                "_VARIABLE_REFRESH",
                "_NET_WM_WINDOW_OPACITY",
                "GAMESCOPE_DISPLAY_EDID_PATH",  # for HDR, I think?
            ]
        ),
    ),
    # Monitor events and queries.
    Category.OUTPUTS: Match(
        log_regexes=[],
        operations=[
            "RANDR",
            "RANDR GetCrtcInfo",
            "RANDR GetOutputInfo",
            "RANDR GetOutputPrimary",
            "RANDR GetOutputProperty",
            "RANDR GetProviders",
            "RANDR GetScreenResourcesCurrent",
            "wl_output",
            "xdg_output",
            "zaura_output",
        ]
        + x11_property_operations(
            [
                "_XWAYLAND_RANDR_EMU_MONITOR_RECTS",
            ]
        ),
    ),
    Category.SYNC: Match(
        log_regexes=[],
        operations=[
            "wl_display.sync",
            "wl_callback",
        ],
    ),
    # Create, destroy, map, parent, set title/icon.
    # Also include _WINE_HWND* since they contain category-crossing
    Category.WINDOW_LIFECYCLE: Match(
        log_regexes=[],
        operations=[
            "CreateWindow",
            "CreateNotify",
            "MapWindow",
            "UnmapWindow",
            "MapNotify",
            "MapRequest",
            "ReparentNotify",
            "ReparentWindow",
            "wl_compositor.create_surface",
            "xdg_wm_base.get_xdg_surface",
            "zaura_shell.get_aura_surface",
            "xdg_surface.get_toplevel",
            "xdg_toplevel.set_title",
            "wl_surface.destroy",
            "zaura_surface.set_startup_id",
            "zaura_surface.set_application_id",
            "zaura_surface.set_fullscreen_mode",
            "BadWindow",
            "ClientMessage WL_SURFACE_ID",
        ]
        + x11_property_operations(
            [
                "WM_CLASS",
                "WM_PROTOCOLS",
                "WM_TRANSIENT_FOR",
                "WM_ICON_NAME",
                "WM_ICON_SIZE",
                "WM_NAME",
                "_NET_WM_NAME",
                "_NET_WM_VISIBLE_NAME",
                "_NET_WM_ICON_NAME",
                "_NET_WM_VISIBLE_ICON_NAME",
                "_NET_WM_ICON",
                "STEAM_GAME",
                "_NET_WM_PID",
                "_NET_WM_HANDLED_ICONS",
                "_NET_WM_USER_TIME",
                "_NET_WM_USER_TIME_WINDOW",
                "_WINE_HWND_STYLE",
                "_WINE_HWND_EXSTYLE",
                "_NET_STARTUP_ID",
                "WM_CLIENT_LEADER",
                "SM_CLIENT_ID",
                "WM_WINDOW_ROLE",
            ]
        ),
    ),
    # Whether a window has a frame, and what those frame decorations look like.
    # The frame size is in WINDOW_RECT because it affects positioning.
    Category.WINDOW_FRAME: Match(
        log_regexes=[],
        operations=[
            "zaura_surface.set_frame",
            "zaura_surface.set_frame_colors",
        ]
        + x11_property_operations(
            [
                "_MOTIF_WM_HINTS",
                "_GTK_THEME_VARIANT",
            ]
        ),
    ),
    # Messages affecting how the compositor or window manager
    # should behave with regard to the window at a high level.
    # Includes window states like fullscreen/maximized/minimized/iconified,
    # window decorations, and other WM hints.
    Category.WINDOW_STATE: Match(
        log_regexes=[
            # Apps might try using override-redirect to request fullscreen.
            r"ConfigureNotify.* override-redirect: ",
        ],
        operations=[
            "SendEvent _NET_WM_STATE",
            "Sent-ClientMessage _NET_WM_STATE",
            "xdg_surface.configure",
            "xdg_surface.ack_configure",
            "xdg_toplevel.set_fullscreen",
            "xdg_toplevel.unset_fullscreen",
            "xdg_toplevel.set_maximized",
            "xdg_toplevel.unset_maximized",
            "xdg_toplevel.set_minimized",
            "xdg_toplevel.configure",
            "xdg_toplevel.configure_bounds",
        ]
        + x11_property_operations(
            [
                "WM_STATE",
                "_NET_WM_STATE",
                "WM_HINTS",
                "_NET_WM_BYPASS_COMPOSITOR",
                "_NET_WM_WINDOW_TYPE",
                "_NET_WM_ALLOWED_ACTIONS",
            ]
        ),
    ),
    # Operations not typically useful for debugging.
    Category.MISC: Match(
        log_regexes=[],
        operations=[
            "GetProperty",  # GetProperty with no atom = a GetProperty reply
        ]
        + x11_property_operations(["WM_CLIENT_MACHINE", "XdndAware"]),
    ),
    # Changes/queries regarding the size/position of a window.
    Category.WINDOW_RECT: Match(
        log_regexes=[
            r"ConfigureNotify.* (x|y|width|height|border-width): ",
            r"ConfigureWindow.* (x|y|width|height|border-width): ",
        ],
        operations=[
            "ConfigureRequest",
            "GetGeometry",
            "GetWindowAttributes",
            "ChangeWindowAttributes",
            "TranslateCoordinates",
            "QueryTree",
            "SendEvent _NET_WM_FULLSCREEN_MONITORS",
            "Sent-ClientMessage _NET_WM_FULLSCREEN_MONITORS",
            "wp_viewporter.get_viewport",
            "wp_viewport.set_destination",
            "wp_viewport.destroy",
            "xdg_surface.configure",
            "xdg_surface.ack_configure",
            "xdg_toplevel.move",
            "xdg_toplevel.resize",
            "xdg_toplevel.set_max_size",
            "xdg_toplevel.set_min_size",
            "xdg_toplevel.configure",
            "xdg_toplevel.configure_bounds",
            "xdg_toplevel.set_fullscreen",
            "xdg_toplevel.unset_fullscreen",
            "xdg_toplevel.set_maximized",
            "xdg_toplevel.unset_maximized",
            "xdg_toplevel.set_minimized",
        ]
        + x11_property_operations(
            [
                "_NET_FRAME_EXTENTS",
                "_NET_WORKAREA",
                "_GTK_WORKAREAS_D0",
                "_GTK_WORKAREAS_D1",
                "_GTK_WORKAREAS_D2",
                "_NET_WM_DESKTOP",
                "_NET_WM_STRUT",
                "_NET_WM_STRUT_PARTIAL",
                "_NET_WM_ICON_GEOMETRY",
                "WM_NORMAL_HINTS",
            ]
        ),
    ),
    # Changes to which window receives input or is displayed on top.
    Category.FOCUS: Match(
        log_regexes=[
            "ConfigureNotify.* above-sibling: ",
            "ConfigureWindow.* stack-mode: ",
        ],
        operations=[
            "Expose",
            "Sent-Expose",
            "SendEvent Expose",
            "FocusIn",
            "FocusOut",
            "GetInputFocus",
            "GrabPointer",
            "UngrabPointer",
            "GrabKeyboard",
            "UngrabKeyboard",
            "EnterNotify",
            "LeaveNotify",
            "SetInputFocus",
            "wl_keyboard.leave",
            "wl_keyboard.enter",
            "zwp_locked_pointer_v1",
            "zwp_pointer_constraints_v1",
        ],
    ),
}

operation_to_categories = {}
compiled_regexes = []


def simple_categorize(log):
    """Categorize the given log statement without using context, if possible.

    Returns a tuple of (category_list, operation) where category_list is a list of categories
    (strings), and operation is a high-level summary of the request, event, or error represented by
    the log statement.
    """
    categories = []

    # Lazy init: Assemble a list of compiled regexes and corresponding categories.
    if not compiled_regexes:
        for category, matcher in category_matchers.items():
            compiled_regexes.extend(
                [(re.compile(r), category) for r in matcher.log_regexes]
            )

    # Assign categories by regex.
    for regex, category in compiled_regexes:
        if regex.search(log):
            categories.append(category)

    # Parse out the operation.
    operation = x11_operation(log) or wayland_operation(log)
    if not operation:
        if not categories:
            print(f"Warning: Unparsed log: {log}")
        return categories, None

    # Lazy init: Invert category_to_operations for quicker lookup.
    if not operation_to_categories:
        for c, m in category_matchers.items():
            for o in m.operations:
                operation_to_categories.setdefault(o, []).append(c)

    # Assign categories based on the full operation.
    categories.extend(operation_to_categories.get(operation, []))

    if not categories:
        # Fallback case: Try using just the first word of the operation.
        word_match = re.match("([^. ]*)", operation)
        if word_match:
            short_operation = word_match.group(1)
            categories.extend(operation_to_categories.get(short_operation, []))

    return categories, operation


class Categorizer:
    """Assigns categories to Wayland or X11 events/requests.

    This class is stateful, to handle cases where context is needed for categorization.

    Each log source should have its own Categorizer, since for example different Wayland
    connections will have clashing IDs.
    """

    def __init__(self):
        # Wayland object deletion events are quite generic. It's most useful to mirror the
        # categories used for the object's creation event, so keep track of those, by object ID.
        self.object_creation_categories_for_id = {}
        # Track operations that weren't matched, for debugging purposes
        self.unmatched_operations = {}

    def categories(self, log):
        """Return a list of categories for the given log entry.

        Args:
            log: A single line of text from a windowing log.
        """
        # wl_display.delete_id events match the category of the request that created that ID.

        cats, op = simple_categorize(log)

        # Some operations copy the category of the request that created a referenced object ID.
        if op in ["wl_display.delete_id", "wl_callback.done"]:
            # Extract the object ID
            object_id = None
            delete_id_match = re.search(
                r"wl_display@[0-9]*.delete_id\(([0-9]*)\)$", log
            )
            if delete_id_match:
                object_id = delete_id_match.group(1)
            else:
                done_match = re.search(r"wl_callback@([0-9]*).done\(", log)
                if done_match:
                    object_id = done_match.group(1)

            # Apply the category from that object's creation event.
            if object_id in self.object_creation_categories_for_id:
                cats = self.object_creation_categories_for_id[object_id]

        # Track creation events for later matching to wl_display.delete_id instances.
        if "new id " in log:
            id_match = re.search(r"new id [\w_]*@([0-9]*)", log)
            new_id = id_match.group(1)
            self.object_creation_categories_for_id[new_id] = cats

        if not cats:
            operation = x11_operation(log) or wayland_operation(log)
            self.unmatched_operations.setdefault(operation, 0)
            self.unmatched_operations[operation] += 1

        return cats, op


def x11_parameter(log, parameter_name):
    """Parse the value of a named parameter from an X11 event/request/reply.

    Sample input:
      "← 28 (PropertyNotify) {... atom: 67 (WM_CLASS); ...}", "atom"

    Sample output:
      WM_CLASS
    """
    parameter_re = rf"{parameter_name}: [0-9][0-9.]* \(([\w <>-]*)\)"
    parameter_match = re.search(parameter_re, log)
    if parameter_match:
        return parameter_match.group(1)
    return None


def x11_operation(log):
    """Parse the operation, and maybe an important parameter, from an X11 event/request/reply.

    Sample inputs:
      "→ 26 (GrabPointer) { ... }"
      "← 52 (<Unknown eventcode 52>) { undecoded }"
      "←⚠️ errorcode: 9 (BadDrawable) { ... }""
      "← 28 (PropertyNotify) {... atom: 67 (WM_CLASS); ...}"

    Sample outputs:
      "GrabPointer"
      "<Unknown eventcode 52>"
      "BadDrawable"
      "PropertyNotify WM_CLASS"
    """
    x11_match = re.match(
        r".(?:.. errorcode:)? [0-9][0-9.]* \(([\w <>-]*)\)", log
    )
    if x11_match:
        operation = x11_match.group(1)
        param = (
            x11_parameter(log, "extension")
            or x11_parameter(log, "extension-minor")
            or x11_parameter(log, "property")
            or x11_parameter(log, "atom")
            or x11_parameter(log, "type")
            or x11_parameter(log, "event")
        )
        if param:
            return f"{operation} {param}"
        return operation

    return None


def wayland_operation(log):
    """Remove IDs, parameters, and direction arrows from a Wayland request/event.

    Sample inputs:
      "→ wl_surface@64.commit()"
      "xdg_toplevel@123.configure(128, 16, array[0])""

    Sample outputs:
      "wl_surface.commit"
      "xdg_toplevel.configure"
    """
    wayland_match = re.match(r" ?→? ?([\w_]*)@[0-9]*\.(\w*)", log)
    if wayland_match:
        return f"{wayland_match.group(1)}.{wayland_match.group(2)}"
    return None
