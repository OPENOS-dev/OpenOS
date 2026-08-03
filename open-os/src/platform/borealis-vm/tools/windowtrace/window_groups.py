# Copyright 2024 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Routines to group X11 and Wayland objects which refer to the same "window".

The user's concept of a singular "window" actually encompasses ~10 different
types of object in our combined X11+Wayland stack. These objects each perform a
different function, or live at a different layer of the stack. Herein, we refer
to each such object as a "window object", and a bunch of related ones as a
"window group".

By scanning the window trace spreadsheet as a post-process, we can recover
the relationships between window objects one at a time, and use them to build
a graph. For example, objects mentioned in the following log lines form a
connected graph:

→ wl_surface@94.attach(wl_buffer@95, 0, 0)
→ wp_viewporter@8.get_viewport(new id wp_viewport@92, wl_surface@94)
→ xdg_wm_base@15.get_xdg_surface(new id xdg_surface@71, wl_surface@94)
→ xdg_surface@71.get_toplevel(new id xdg_toplevel@66)
→ zaura_shell@11.get_aura_surface(new id zaura_surface@74, wl_surface@94)
→ wl_surface@94.frame(new id wl_callback@86)

It's also useful to recover the window title. If we consider the title
as a special kind of "window object", we can add it to the graph, with
an edge to the window object that stores the name. For example, for Wayland
we can build an edge between an xdg_toplevel and the title when we observe
a log message like this one:

→ xdg_toplevel@66.set_title("My First Game")

Not all Wayland objects should be connected in this way. In particular, we should
ignore globals like wl_compositor, xdg_wm_base, and zura_shell, and
non-window objects like wl_pointer and wl_keyboard.

If multiple window groups exist, each will therefore form a connected component
of the final graph. The Window column in the trace spreadsheet will be populated
by descriptions of window groups referenced by each log line.
"""

import re

from windowtrace import categories
from windowtrace import x11_parse


# Types of Wayland object that can be part of a window group.
WANTED_WAYLAND_TYPES = {
    "wl_buffer",
    "wl_callback",
    "wl_region",
    "wl_shell_surface",
    "wl_subsurface",
    "wl_surface",
    "wp_viewport",
    "xdg_positioner",
    "xdg_surface",
    "xdg_toplevel",
    "zaura_popup",
    "zaura_surface",
    "zaura_toplevel",
}


class Node:
    """Represents one "window object" in the graph of grouped window objects.

    Rather than model the graph directly, we use a disjoint-set forest
    (https://en.wikipedia.org/wiki/Disjoint-set_data_structure) to track connected components.
    The edges of the graph are not stored, as they are not needed.
    """

    def __init__(self, name, is_host, generation):
        # String identifier of this node, such as "wl_surface@10",
        # "0x00200011", or "My First Game".
        self.name = name

        # Used to visually differentiate objects with the same name
        # but different identities.
        self.generation = generation

        # True if the object exists in a host-side connection.
        #
        # This differentiates Wayland objects on the Xwayland side of Sommelier
        # from those on the Exo side of Sommelier, as they exist on different
        # connections and therefore in different namespaces.
        self.is_host = is_host

        # Window title (a string) assigned to this node, if any.
        self.title = None

        # An arbitrary node belonging to the same window group,
        # or None if this node is presently the root of its window group tree.
        #
        # The order of the parent/child relationship is arbitrary, and (counter-intuitively) this
        # node and its parent don't necessarily have a direct relationship. It's merely used to
        # discover the "connected components" of the graph.
        self.parent = None

        # If parent is None (this node is a root), this is the height of this node's tree.
        self.height = 0

    def __str__(self):
        parent = f"→ {str(self.parent)}" if self.parent else "<root>"
        connection = "host" if self.is_host else "guest"
        return f"{self.name} [{connection}] {parent}"

    def is_visible_in_window_column(self):
        """True if this node's name should be shown in the final trace output."""
        # Omit shortlived objects like wl_buffer and wl_callback.
        # We still track these, so that for example a wl_callback.done event will
        # be tagged to the correct window, we just don't list them since it'd be spammy.
        if (self.name.startswith("wl_buffer@")) or (
            self.name.startswith("wl_callback@")
        ):
            return False
        return True

    def display_name(self):
        """Name of node, to be displayed in the final trace output."""
        host = " [host]" if self.is_host else ""
        generation = (
            chr(ord("a") + (self.generation % 26))
            if self.generation > 0
            else ""
        )
        return f"{self.name}{generation}{host}"

    def root(self):
        """Return the root node of the tree this node is in."""
        n = self
        while n.parent:
            n = n.parent
        return n

    def merge(self, other):
        """Merge self and other into the same tree, if they're not already."""
        a = self.root()
        b = other.root()
        if a == b:
            return
        if a.height < b.height:
            lower = a
            higher = b
        else:
            lower = b
            higher = a
        lower.parent = higher
        if lower.height == higher.height:
            higher.height += 1


def live_objects_key(source, name):
    """Compute a key for this object in the live_objects map.

    Args:
        source: Differentiates the host-side Wayland connection
            (between Exo and Sommelier) from the guest-side (between Sommelier and Xwayland).
        name: Wayland object name, or X11 window ID.
    """
    # live_objects keys omit the Wayland type, to aid in tracking lifetimes.
    name_without_type = name.split("@")[-1] if "@" in name else name
    is_host = "Exo" in source
    connection = "host" if is_host else "guest"
    return f"{connection}|{name_without_type}"


class WindowGrouper:
    """Parses LogLines to create and assign window groups.

    First, call parse() on each LogLine, in ascending order of their timestamps.

    Second, call populate_window_columns(). This fills the `window` member of
    previously parsed LogLines.
    """

    def __init__(self):
        self.all_nodes = []
        self.window_group = {}

        # Map from IDs to Nodes for any objects we've found which haven't yet been deleted.
        # For Wayland objects these are numeric identifiers without the type prefix,
        # as passed to delete_id messages.
        self.live_objects = {}

        # Remember the most recent Wayland request received by Sommelier from Xwayland
        # (in the sommelier → XWayland column). We'll attempt to match this with
        # a similar outgoing request to Exo, and thus infer the relationship
        # between wl_surface objects on the host and guest sides.
        self.last_sommelier_wayland_request = None
        self.last_sommelier_wayland_request_node = None

        # Keys are host-side wl_surface nodes.
        # Values are maps from guest-side wl_surface nodes to a count of times that
        # these host-side and guest-side nodes appeared to be related.
        self.inferred_guest_surface_counts = {}

    def node(self, source, name, generation, create=True):
        """Return a new or existing node with the given source and name."""
        key = live_objects_key(source, name)
        n = self.live_objects.get(key)

        # Create Nodes that don't already exist.
        if not n and create:
            is_host = "Exo" in source
            n = Node(name, is_host, generation)
            self.live_objects[key] = n
            self.all_nodes.append(n)

        assert not n or n.name == name
        return n

    def delete_node(self, source, name):
        """Mark the specified node as deleted by removing it from live_objects.

        It will still exist in all_nodes and window_groups.

        Returns -1 if the specified node doesn't exist or was already deleted,
        or else the generation of the deleted node.
        """
        n = self.live_objects.pop(live_objects_key(source, name), None)
        return n.generation if n else -1

    def parse(self, logline):
        """Parse the given LogLine. Must be called on each LogLine, in order."""

        wayland_objects = re.findall(
            r"\b[a-z]+_[a-z0-9]+@[0-9]+\b", logline.text
        )
        nodes = []
        if wayland_objects:
            # Newly-allocated objects should get a new identity,
            # even if their IDs were previously used.
            #
            # On the guest side, sommelier → XWayland and Xwayland → sommelier
            # have duplicate content, and we don't want to treat the duplicate line as
            # reallocating the object that was just reallocated on the previous line.
            # Therefore, look for the "send arrow" → to identify that previous line.
            # The line without the arrow is receiving the message, so should always come later.
            sending = "→" in logline.text
            is_host = "Exo" in logline.source
            generation = 0
            if is_host or sending:
                for o in wayland_objects:
                    if f"new id {o}" in logline.text:
                        generation = self.delete_node(logline.source, o) + 1

            # Create nodes for interesting types.
            nodes = [
                self.node(logline.source, o, generation)
                for o in wayland_objects
                if (o.split("@")[0] in WANTED_WAYLAND_TYPES)
            ]
            # Remember window titles
            if len(nodes) == 1 and "set_title" in logline.text:
                m = re.search(r""".*set_title\("(.*)"\)""", logline.text)
                if m:
                    nodes[0].title = str(m.group(1))

            # Group surfaces across the guest/host boundary.
            if nodes and logline.source.startswith("sommelier"):
                surface_nodes = [
                    n for n in nodes if n.name.startswith("wl_surface@")
                ]
                if len(surface_nodes) == 1:
                    if not is_host and not sending:
                        # This is a request from XWayland, as logged by sommelier.
                        self.last_sommelier_wayland_request = logline
                        self.last_sommelier_wayland_request_node = (
                            surface_nodes[0]
                        )
                    elif (
                        is_host
                        and sending
                        and self.last_sommelier_wayland_request_node
                    ):
                        # This is a request to Exo, as logged by sommelier.
                        # If it's similar to last_sommelier_wayland_request, we can use it to infer
                        # which wl_surfaces are related between the guest side and the host side.
                        guest_logline = self.last_sommelier_wayland_request
                        guest_node = self.last_sommelier_wayland_request_node
                        last_guest_operation = categories.wayland_operation(
                            guest_logline.text
                        )
                        this_operation = categories.wayland_operation(
                            logline.text
                        )
                        if this_operation == last_guest_operation:
                            # Count the number of times these surfaces appeared linked.
                            m = self.inferred_guest_surface_counts.setdefault(
                                surface_nodes[0], {}
                            )
                            m[guest_node] = m.get(guest_node, 0) + 1

                            # We matched this request already, don't match it again.
                            self.last_sommelier_wayland_request_node = None
        else:
            operation = categories.x11_operation(logline.text)
            params = x11_parse.parameters(logline.text)

            # TODO(cpelling): Do we need "generation" for reused X11 IDs?

            # Link the X11 frame window (the one Sommelier creates) to the guest-side wl_surface.
            # TODO(b/288022332): Support https://wayland.app/protocols/xwayland-shell-v1 if
            # Sommelier ever transitions to using it.
            if operation == "ClientMessage WL_SURFACE_ID":
                window = x11_parse.window_param(params, "eventwindow")
                if window and len(params.get("data", [])) == 1:
                    wayland_id = x11_parse.decode_hex_data_as_int(
                        params["data"][0]
                    )
                    assert wayland_id < (1 << 32)
                    nodes = [
                        self.node(
                            logline.source, f"wl_surface@{wayland_id}", 0
                        ),
                        self.node(logline.source, window, 0),
                    ]
            elif operation == "ReparentWindow" and logline.source.startswith(
                "sommelier"
            ):
                # Link reparented windows to their frame windows.
                window = x11_parse.window_param(params, "window")
                parent = x11_parse.window_param(params, "parent")
                if window and parent:
                    nodes = [
                        self.node(logline.source, window, 0),
                        self.node(logline.source, parent, 0),
                    ]
            else:
                # Tag logs that reference window IDs, but only one window at a time,
                # because in the general case merely mentioning two X11 windows together
                # shouldn't automatically link them into one window group.
                for window_parameter_name in [
                    "wid",
                    "window",
                    "eventwindow",
                    "drawable",
                ]:
                    window_id = x11_parse.window_param(
                        params, window_parameter_name
                    )
                    if window_id:
                        # Don't create a window node for "drawable" parameters unless we
                        # already know that this is a window ID; because although all windows
                        # are drawables, some drawables are not windows.
                        create = window_parameter_name != "drawable"

                        n = self.node(
                            logline.source, window_id, 0, create=create
                        )
                        if n:
                            nodes.append(n)

                            # Take the opportunity to tag the window with a title.
                            n.title = x11_parse.title_from_window_param(
                                params, window_parameter_name
                            )

                        # Finding one window ID is enough; we don't always want to link
                        # windows IDs that appear together.
                        break

        if not nodes:
            return

        self.window_group[logline] = nodes[0]

        # Iterate all pairs of found window objects and merge them into the same set.
        for i, first in enumerate(nodes):
            for second in nodes[i + 1 :]:
                first.merge(second)

    def populate_window_columns(self):
        """Assign `window` on each previously parsed LogLine."""

        # Infer guest-host surface relationships.
        # Greedily picking surfaces most frequently seen together seems to work fine.
        guest_nodes_picked = set()
        for host_node, counts in self.inferred_guest_surface_counts.items():
            guest_node = max(counts.items(), key=lambda item: item[1])[0]

            # Guard against this greedy algorithm failing by wrongly associating
            # a guest wl_surface with more than one host wl_surface.
            assert guest_node not in guest_nodes_picked
            guest_nodes_picked.add(guest_node)

            # This host wl_surface appears related to this guest wl_surface, so merge them into
            # the same set.
            host_node.merge(guest_node)

        # Build a list of all the nodes in each tree.
        # Keys are tree roots.
        nodes_in_tree = {}
        for n in self.all_nodes:
            nodes_in_tree.setdefault(n.root(), []).append(n)

        # Format content for the Window column based on those lists.
        window_text = {}
        for root, nodes in nodes_in_tree.items():
            titles = {n.title for n in nodes if n.title is not None}
            title_text = "/".join(titles)
            if title_text:
                title_text += ": "
            names = [
                n.display_name()
                for n in nodes
                if n.is_visible_in_window_column()
            ]
            name_text = ", ".join(sorted(names, reverse=True))
            window_text[root] = title_text + name_text

        # Fill in that content to the Window cell of each log line.
        for logline, node in self.window_group.items():
            logline.window = window_text[node.root()]
