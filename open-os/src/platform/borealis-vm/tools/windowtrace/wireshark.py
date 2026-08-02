# Copyright 2022 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Routines for reformatting Wireshark's decoding of X11 packets."""

import re
import struct

from . import x11_parse


# tshark's output can be verbose, so abbreviate some common prefixes.
# For example, we denote Requests with a right arrow.
# Replacements are done in order, incorporating previous replacements
# into the input string.
X11_PACKET_PREFIX_REPLACEMENTS = [
    ("X11, ", ""),
    ("Request, ", "→ "),
    ("Reply, ", "← "),
    ("Event, ", "← "),
    ("Error, ", "←⚠️ "),
    ("→ opcode: ", "→ "),
    ("← opcode: ", "← "),
    ("← eventcode: ", "← "),
]

UNKNOWN_ATOM = "(Not a predefined atom)"

WM_SIZE_HINTS_FLAGS = [
    ("USPosition", 1 << 0),
    ("USSize", 1 << 1),
    ("PPosition", 1 << 2),
    ("PSize", 1 << 3),
    ("PMinSize", 1 << 4),
    ("PMaxSize", 1 << 5),
    ("PResizeInc", 1 << 6),
    ("PAspect", 1 << 7),
    ("PBaseSize", 1 << 8),
    ("PWinGravity", 1 << 9),
]

WM_SIZE_HINTS_LABELS = [
    "flags",
    "x",
    "y",
    "width",
    "height",
    "min_width",
    "min_height",
    "max_width",
    "max_height",
    "width_inc",
    "height_inc",
    "min_aspect_x",
    "min_aspect_y",
    "max_aspect_x",
    "max_aspect_y",
    "base_width",
    "base_height",
    "win_gravity",
]


def parse_inside_parens(s):
    """Return text inside parentheses.

    Returns:
        A substring of the input, between the first opening parenthesis and the
        last closing parenthesis.

        For example, given '0x000e (My Window)' returns 'My Window'.
    """
    start = s.find("(")
    end = s.rfind(")", start)
    if start != -1 and end != -1:
        return s[start + 1 : end]
    return ""


def massage_x11_packet(packet, atoms, windownamesbyid):
    """Abbreviate Wireshark's decoding of a single X11 packet.

    windownamesbyid is mutated in-place, and used here to annotate
    window IDs with their known names. Consider it opaque to the caller.
    """

    # First, replace some common prefixes.
    for r in X11_PACKET_PREFIX_REPLACEMENTS:
        if packet.startswith(r[0]):
            packet = r[1] + packet.removeprefix(r[0])

    # Remove uninteresting lines; unused data, opcodes, event codes,
    # packet/string lengths, bitmask breakdowns, and configure-window-mask
    # (which Wiresharks decodes inaccurately); and split by lines.
    def is_interesting(line):
        l = line.strip()
        return (
            l not in ("unused", "eventcode: 35 (GenericEvent)")
            and not l.startswith("data-length: ")
            and not l.startswith("name-length: ")
            and not l.startswith("replylength: ")
            and not l.startswith("request-length: ")
            and not l.startswith("opcode: ")
            and not l.startswith("eventcode: ")
            and not l.startswith("configure-window-mask: ")
            and ".... .... " not in l
        )

    lines = [l for l in packet.splitlines() if is_interesting(l)]

    # Replace atoms unknown to Wireshark with their actual values.
    def fill_in_atom(m):
        atom = int(m.group(1))
        name = atoms.get(atom, "<unknown atom>")
        return f"{atom} ({name})"

    for i, line in enumerate(lines):
        if UNKNOWN_ATOM in line:
            lines[i] = re.sub(
                r"(\d+) \(Not a predefined atom\)", fill_in_atom, line
            )

    # Augment Wireshark's annotation of certain packets to reduce manual toil.
    annotate_change_property_requests(lines, atoms, windownamesbyid)
    annotate_wm_state_client_messages(lines, atoms)

    # Name windows with known names.
    for i, line in enumerate(lines):
        if ("window: " in line) or ("drawable: " in line):
            fields = line.split(": ", maxsplit=1)
            if fields[1] in windownamesbyid:
                lines[i] += f" ({windownamesbyid[fields[1]]})"

    # Wireshark gives us nice verbose multi-line packet breakdowns.
    # For our purposes, a single succinct line is a better fit, so reformat
    # into one line.
    line = indents_to_braces(lines)

    return line


def enumerated_field_values(lines, key):
    """Given a list of 'key: value' strings, find indices and values for the given key.

    For example, given lines = ['foo: a', 'bar: b', 'foo: c'] and key = 'foo',
    returns [(0, 'a'), (2, 'c')].
    """

    def has_key(line, key):
        """True if the line contains a 'key: value' pair with the named key."""
        return line.lstrip().startswith(f"{key}: ")

    def value(line):
        """Return the value from a colon-separated key/value pair string."""
        return line.split(": ", maxsplit=1)[1]

    return [(i, value(l)) for (i, l) in enumerate(lines) if has_key(l, key)]


def count_leading_spaces(s):
    """Return the number of spaces at the beginning of the given string."""
    return len(s) - len(s.lstrip())


def annotate_change_property_requests(lines, atoms, windownamesbyid):
    """Make ChangeProperty requests more human-readable."""
    if not lines[0].endswith("(ChangeProperty)"):
        return

    # Find the type of the property data.
    # There should be exactly one 'type' field.
    typefields = enumerated_field_values(lines, "type")
    if len(typefields) != 1:
        return
    typeidx, typefield = typefields[0]

    if typefield.endswith("(ATOM)"):
        # Annotate ATOM property values with atom names.
        #
        # Wireshark writes out an array of "item" fields with values in hexadecimal,
        # so simply convert to integer and do a lookup.
        for i, atomhexstr in enumerated_field_values(lines, "item"):
            atom = int(atomhexstr, 16)
            name = atoms.get(atom, "<unknown atom>")
            lines[i] += f" ({name})"

    elif typefield.endswith("(WM_SIZE_HINTS)"):
        # Parse out and describe the WM_SIZE_HINTS struct. Reference:
        # https://tronche.com/gui/x/xlib/ICC/client-to-window-manager/wm-normal-hints.html
        # First, convert the item field values to ints.
        # They're in hex, so use int(item, 16) to parse them.
        data = [
            int(item, 16)
            for idx, item in enumerated_field_values(lines, "item")
        ]

        # Prepare to insert the struct description immediately after the type field,
        # using the same indentation level.
        indent = " " * count_leading_spaces(lines[typeidx])
        output = [f"{indent}XSizeHints"]

        # Each element in `data` is a struct member.
        for i, d in enumerate(data):
            # WM_SIZE_HINTS_LABELS names all the struct members, in order.
            label = (
                WM_SIZE_HINTS_LABELS[i]
                if i < len(WM_SIZE_HINTS_LABELS)
                else "???"
            )
            if label == "flags":
                # WM_SIZE_HINTS_FLAGS enumerates the flag names and bitmasks
                # used to interpret the "flags" field.
                flags = [
                    name
                    for (name, bitmask) in WM_SIZE_HINTS_FLAGS
                    if (d & bitmask) == bitmask
                ]
                val = "|".join(flags)
            else:
                # Other fields just have numeric values.
                val = d
            output.append(f"{indent}    {label}: {val}")
        lines[typeidx + 1 : typeidx + 1] = output

    elif typefield.endswith("(STRING)") or typefield.endswith("(UTF8_STRING)"):
        # For string types, Wireshark outputs a single hex-encoded binary data field.
        # For example, "hello" becomes "68656c6c6f".
        fields = enumerated_field_values(lines, "data")
        if fields:
            idx, data = fields[0]
            binarydata = x11_parse.decode_hex_data(data)

            # The STRING type uses ISO-8859-1 encoding (Latin-1).
            encoding = "latin_1" if typefield.endswith("(STRING)") else "utf8"

            # Some fields, like WM_CLASS, use null bytes, so replace those with
            # a human-readable sentinel.
            stringdata = binarydata.decode(encoding, errors="replace").replace(
                "\0", "<NUL>"
            )

            lines[idx] += f' "{stringdata}"'

            # As a side-effect, remember window names for later use.
            properties = enumerated_field_values(lines, "property")
            if properties:
                _, prop = properties[0]
                if prop.endswith("(WM_NAME)") or prop.endswith(
                    "(_NET_WM_NAME)"
                ):
                    windows = enumerated_field_values(lines, "window")
                    if windows:
                        _, window = windows[0]
                        windownamesbyid[window] = stringdata


def annotate_wm_state_client_messages(lines, atoms):
    """Decode client messages of type _NET_WM_STATE."""
    if not lines[0].endswith("(SendEvent)"):
        return

    eventfields = enumerated_field_values(lines, "event")
    if len(eventfields) != 1 or eventfields[0][1] != "33 (ClientMessage)":
        return

    typefields = enumerated_field_values(lines, "type")
    if len(typefields) != 1:
        return
    _, eventtype = typefields[0]

    if parse_inside_parens(eventtype) != "_NET_WM_STATE":
        return

    datafields = enumerated_field_values(lines, "data")
    if len(datafields) != 1:
        return

    idx, data = datafields[0]
    binarydata = x11_parse.decode_hex_data(data)
    try:
        decode = struct.unpack("IIII", binarydata[:16])
    except Exception as e:
        raise ValueError(binarydata) from e

    action = ""
    if decode[0] == 0:
        action = "_NET_WM_STATE_REMOVE"
    elif decode[0] == 1:
        action = "_NET_WM_STATE_ADD"
    elif decode[0] == 2:
        action = "_NET_WM_STATE_TOGGLE"

    prop1 = atoms.get(decode[1], "<unknown atom>")
    prop2 = atoms.get(decode[2], "<unknown atom>") if decode[2] != 0 else "0"

    source = ""
    if decode[3] == 0:
        source = "source = unspecified"
    elif decode[3] == 1:
        source = "source = normal app"
    elif decode[3] == 2:
        source = "source = pager"

    lines[
        idx
    ] += f" ({decode[0]} [{action}], {prop1}, {prop2}, {decode[3]} [{source}])"


def indents_to_braces(lines):
    """Format a list of indented strings into one line, with C-style brackets.

    Preserves structure by replacing indents with bracketing. For example,
    given input:

    [
        'one',
        '    two',
        '        three',
        '        four',
        '            five',
        '            six',
    ]

    Outputs: 'one { two { three; four { five; six }}}'
    """

    # Stack of indentation levels.
    indents = []
    # Append a blank line to the input to ensure trailing braces are added.
    lines.append("")

    result = []
    for l in lines:
        indent = len(l) - len(l.lstrip())
        if len(indents) == 0:
            # No delimiters at the start of the output string.
            # Just record the indentation level for next time.
            indents.append(indent)
        elif indent > indents[-1]:
            # Indent is increasing; delimit with opening brace.
            result.append(" { ")
            indents.append(indent)
        elif indent == indents[-1]:
            # Same indent as previous line; delimit with semicolon.
            result.append("; ")
        else:
            # Indent is decreasing; delimit with closing braces.
            braces = ""
            while len(indents) > 0 and indent < indents[-1]:
                braces += "}"
                indents.pop()
            result.append(f" {braces} ")

        # Add the input line to the output, with indentation removed.
        result.append(l.strip())

    return "".join(result)
