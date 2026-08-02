# Copyright 2024 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Common routines for parsing X11 logs, as generated via Wireshark."""

import re
import string


def decode_hex_data(data):
    """Convert hex-string-encoded binary data to a bytes object.

    For example, for the string "4368726f6d65626f6f6b" return b"Chromebook";
    noting 0x43 represents the ASCII character 'C', 0x68 is 'h', and so on.
    """

    def hex_str_to_bytes(h):
        try:
            return bytes([int(h, 16)])
        except ValueError:
            return bytes(f"<Invalid hex string: {h}>", encoding="utf8")

    return b"".join(
        hex_str_to_bytes(data[i : i + 2]) for i in range(0, len(data), 2)
    )


def decode_hex_data_as_int(data):
    """Convert hex-string-encoded binary data to a little-endian integer.

    For example, for the string "3400000000000000" return 0x34 (52).
    """
    return int.from_bytes(decode_hex_data(data), "little")


def parameters(log):
    """Given an X11 log message, return a map of parameter names to lists of associated values."""
    brace_start = log.find("{")
    brace_end = log.rfind("}", brace_start + 1)
    if brace_start == -1 or brace_end == -1:
        return {}

    params = {}
    paren_depth = 0
    in_string = False
    token = ""
    key = None
    for c in log[brace_start + 1 : brace_end]:
        if c == '"':
            in_string = not in_string
            token += c
        elif in_string:
            token += c
        elif c == "(":
            paren_depth += 1
            token += c
        elif c == ")":
            paren_depth -= 1
            token += c
        elif paren_depth > 0:
            token += c
        elif c == ":" and token:
            key = token
            token = ""
        elif c == ";" and key:
            # For atom values with a known name, strip the atom number
            m = re.match(r"[0-9][0-9.]* \(([\w <>-]*)\)", token)
            if m:
                token = m.group(1)

            params.setdefault(key, []).append(token)
            key = None
            token = ""
        else:
            # Skip leading whitespace
            if token or c != " ":
                token += c

    # Handle the last key/value pair.
    if key:
        params.setdefault(key, []).append(token.rstrip())

    return params


def window_param(params, field_name):
    """Parse a window ID from the named field in the given parameter map."""
    p = params.get(field_name, [])
    if len(p) == 1:
        # Trim off the window name which sometimes appears in parentheses.
        wid = p[0][:10]
        if is_x11_window_id(wid):
            return wid
    return None


def title_from_window_param(params, field_name):
    """Parse a window title from the named field in the given parameter map.

    The expected format of the field is "0x03800001 (Game Title)"
    and we want to return "Game Title".
    """
    if window_param(params, field_name):
        field = params[field_name][0]
        if len(field) > 13 and field[11] == "(" and field[-1] == ")":
            return field[12:-1]
    return None


def is_x11_window_id(window_id):
    """Return true if the associated window ID looks like an X11 window ID."""
    assert isinstance(window_id, str)
    return (
        len(window_id) == 10
        and window_id.startswith("0x")
        and all(c in string.hexdigits for c in window_id[2:])
    )
