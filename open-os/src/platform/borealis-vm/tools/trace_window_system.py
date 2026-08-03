#!/usr/bin/env python3
# Copyright 2021 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Analyze a windowtrace*.tar.gz package from usr/bin/trace_window_system.sh.

Run tests (from this working directory):
  python -m unittest trace_window_system.py windowtrace.test_categories
"""

import argparse
import contextlib
import csv
import io
import operator
import os
import re
import subprocess
import sys
import tarfile
import tempfile
from typing import NamedTuple
import unittest

from windowtrace import categories
from windowtrace import visualizer
from windowtrace import window_groups
from windowtrace import wireshark


class LogLine:
    """A single line from a log file.

    Each source log is parsed to produce zero or more LogLines, which are
    initially stored in Columns (one for each column in the output .csv).

    To render the output, we annotate each LogLine with the numeric index of
    its Column, combine all of the LogLines into a single big list, and sort by
    timestamp. Then we just write all the LogLines to the file in order.
    The column index determines the indentation of each line.
    """

    def __init__(self, timestamp, text):
        # Time in milliseconds.
        self.timestamp = timestamp
        # The text to display.
        self.text = text
        # For assignment by WindowGrouper.
        self.window = ""
        # The column index and source are filled out when we render.
        # Give them sentinel values for now.
        self.columnindex = -1
        self.source = ""

    def tocsvrow(self, timestampoffset, categorizers_by_column_idx):
        """Formats this log line as a list for writing to a CSV file.

        timestampoffset: Value to be added to this line's timestamp. Intended
            use is for shifting all the timestamps so that the first row is at
            time 0.
        """
        # Each log source, and therefore each column, needs its own Categorizer.
        c = categorizers_by_column_idx.setdefault(
            self.columnindex, categories.Categorizer()
        )
        category_list, operation = c.categories(self.text)

        # Multiple categories might be assigned to a single log. For now, handle this
        # by creating new categories that combine the existing ones. For example,
        # a ConfigureEvent might be given the category "Window size, Window state".
        # In principle this causes a combinatoric explosion of categories, but in
        # practice we don't see too many.
        category_text = ", ".join(sorted(category_list))

        # Timestamp, Notes, Category, Operation, and Source columns
        row = [
            self.timestamp + timestampoffset,
            "",
            category_text,
            operation,
            self.window,
            self.source,
        ]
        # Indent
        row += [""] * self.columnindex
        # Content
        row.append(self.text)
        return row


class Column:
    """Represents one log source, formatted as a column of the output file."""

    def __init__(
        self, header, short_description, headertitle, loglines, valid=True
    ):
        # Text to display in the column's header cell.
        self.header = header
        # Text to display in the "Source" column, to permit filtering out entire log sources.
        self.short_description = short_description
        # Text to display under the header cell.
        # Originally this was supposed to be a tooltip.
        self.headertitle = headertitle
        # List of LogLine instances to display in this column.
        self.loglines = loglines
        # Only "valid" columns are included in the final output.
        self.valid = valid


def extract_pid(filename):
    """Returns the integer Process ID embedded in the given filename.

    For example, returns 264 when passed
    "tmp/wayland-debug/client-pid264-0x5ccf8b815c88.log".

    Returns None if the string doesn't contain "pid<number>" exactly once.
    """
    matches = re.findall(r"pid(\d+)", filename)
    if len(matches) == 1:
        return int(matches[0])
    return None


def render(columns, visualizer_frames, output_filename):
    """Renders analyzed column data to CSV.

    Args:
        columns: List of all Column objects returned from "analyze" functions.
        visualizer_frames: List of Frame objects from the visualizer module. Can be empty.
        output_filename: The .csv filename to write the output to.
    """
    # All the LogLines from all the columns.
    rows = []
    # Column headers, in display order.
    headers = []
    # Column header tooltips, in display order.
    headertitles = []
    # Categorizer helper objects, by column index.
    categorizers_by_column_idx = {}

    # Pivot from column-major order ('columns') to row-major order ('rows').
    for i, column in enumerate(columns):
        headers.append(column.header)
        headertitles.append(column.headertitle)

        for line in column.loglines:
            # Remember which column this line came from.
            line.columnindex = i
            line.source = column.short_description
            rows.append(line)

    if len(rows) == 0:
        print("Error: No data to render.")
        return

    # Sort all the log lines by their timestamps.
    rows.sort(key=operator.attrgetter("timestamp"))

    # Assign window groups (must be done in its own pass).
    window_grouper = window_groups.WindowGrouper()
    for l in rows:
        window_grouper.parse(l)
    window_grouper.populate_window_columns()

    # Show timestamps relative to the beginning of the trace.
    offset = -rows[0].timestamp

    visualizer_filename = os.path.splitext(output_filename)[0] + ".html"
    frame = visualizer_frames[0] if visualizer_frames else None
    frames = [frame] if frame else []

    with open(output_filename, "w", newline="", encoding="utf-8") as out:
        first_row = [
            "Time (ms)",
            "Notes",
            "Category",
            "Operation",
            "Window",
            "Source",
        ] + headers
        second_row = [
            "",
            "TIP: Make notes in this column.\n"
            "Ctrl+up/Ctrl+down navigates to non-empty cells.",
            "",
            "",
            "",
            "",
        ] + headertitles
        assert len(first_row) == len(second_row)

        writer = csv.writer(out)
        writer.writerow(first_row)
        writer.writerow(second_row)
        for l in rows:
            csvrow = l.tocsvrow(
                offset,
                categorizers_by_column_idx,
            )
            writer.writerow(csvrow)

    if visualizer_frames:
        frames.extend(visualizer_frames[1:])
    with open(visualizer_filename, "w", encoding="utf-8") as f:
        f.write(visualizer.fill_html_template(visualizer_filename, frames))

    # Print a list of operations that weren't matched.
    unmatched_operations = {}
    for c in categorizers_by_column_idx.values():
        for k, v in c.unmatched_operations.items():
            unmatched_operations.setdefault(k, 0)
            unmatched_operations[k] += v

    if unmatched_operations:
        print(
            "\nNote: Some log entries were left uncategorized (blank category):\n"
        )
        print("\tOperation\tCount")
        print("\t---------\t-----")
        for operation, count in sorted(
            unmatched_operations.items(), key=lambda x: -x[1]
        ):
            print(f"\t{operation}\t{count}")

    vis_url = f"file://{os.path.abspath(visualizer_filename)}"
    print(
        f"\nSuccess: Wrote {len(rows)} rows to CSV file {output_filename}"
        f"\n         and {len(frames)} frames to visualizer at {vis_url}"
    )


def analyze_wayland_trace(filename, f, pids):
    """Return Columns containing data from the given Wayland log file.

    filename is similar to "tmp/wayland-debug/client-pid264-0x5ccf8b815c88.log"

    pids is expected to be a mapping from Process IDs to command lines which
    contains the Process ID embedded in the filename (264 in this example).
    """
    log = []

    # Header is the executable name in square brackets,
    # pulled from the pids mapping.
    pid = extract_pid(filename)
    cmdline = pids.get(pid, "").split("\t")
    if len(cmdline) > 0:
        short_description = os.path.basename(cmdline[0])
        header = f"[{short_description}]"
    else:
        # Fallback if the pids mapping is incomplete.
        short_description = f"PID {pid}"
        header = short_description

    # Append info about the likely peer to the header,
    # using domain knowledge.
    if "wayland-debug/server" in filename:
        if header == "[sommelier]" and "-X" in cmdline:
            # This is the X-forwarding version of Sommelier,
            # which only serves Wayland connections to XWayland.
            header += "\n→ XWayland"
            short_description += " → XWayland"
        else:
            # Non-X versions of Sommelier communicate with arbitrary Wayland
            # clients.
            header += "\n→ a Wayland client"
            short_description += " → a Wayland client"
    elif "wayland-debug/client" in filename:
        if header == "[sommelier]":
            # Sommelier is a Wayland client of Exo, only.
            header += "\n→ Exo"
            short_description += " → Exo"
        elif any(True for process in pids.items() if "sommelier" in process[1]):
            # If Sommelier is running, assume it's the only Wayland server
            # in the VM.
            header += "\n→ Sommelier"
            short_description += " → sommelier"
        else:
            # Client is connected to an unknown Wayland server.
            header += "\n→ Wayland server"
            short_description += " → Wayland server"

    # Iterate through the lines in the .log file.
    #
    # Here's a sample line. The timestamp is in square brackets, followed by a
    # a space. We don't attempt to parse the rest of the line:
    #
    # [3779913.050] wl_surface@13.attach(wl_buffer@10, 0, 0)
    with io.TextIOWrapper(f, encoding="utf8") as textfile:
        for l in textfile:
            # Split the timestamp from the rest.
            segments = l.split("] ", maxsplit=1)
            if len(segments) != 2:
                continue

            # Sometimes we see null bytes at the start of the file, due
            # to faulty truncation logic in the collection script.
            # Remove these. (Even if we fix the collection script, we
            # still want to be able to analyze historical traces.)
            timestamp_str = segments[0].lstrip("\x00")

            # Sometimes the first line is truncated. If the timestamp
            # isn't preceded by an opening bracket, assume the line is
            # truncated and skip it.
            if not timestamp_str.startswith("["):
                continue

            try:
                # Remove bracket delimiters and spaces and parse to float.
                timestamp_ms = float(timestamp_str.strip("[] "))
            except ValueError:
                continue

            # Use fancy Unicode arrows instead of boring ASCII ones.
            text = segments[1].rstrip().replace(" -> ", " → ", 1)

            log.append(LogLine(timestamp_ms, text))

    # The tooltip shows the filename of the source log, and the command line
    # of the process that loaded libwayland.
    tooltip = "%s\n\n%s" % (filename, "\n".join(cmdline))

    return [Column(header, short_description, tooltip, log)]


class X11AnalysisPipeline:
    """Convert X11 protocol bytes to human-readable form using Wireshark utilities.

    See analyze_x11_strace_log() for the high-level strategy.
    """

    def __init__(self, header, short_description, subheader):
        self.header = header
        self.short_description = short_description
        self.subheader = subheader
        # pylint: disable=consider-using-with
        self.stdout = tempfile.TemporaryFile()
        # First, use text2pcap to convert a text-formatted hexdump into a
        # binary pcap file with faked TCP headers.
        # pylint: disable=consider-using-with
        self.text2pcap = subprocess.Popen(
            [
                "text2pcap",
                # Fake a source TCP port of 6000 (the default port for
                # X11 over TCP). The destination port is 1234 (arbitrary).
                "-T6000,1234",
                # Lets us denote packet direction using I (input) or O (output).
                "-D",
                # Quiet (less output to stderr).
                "-q",
                # Indicates that input lines begin with a timestamp, in seconds
                # since the Unix epoch. %f denotes the subsecond
                # component, as required by text2pcap.
                "-t",
                "%s.%f",
                # Read input from stdin, write output to stdout.
                "-",
                "-",
            ],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.DEVNULL,
        )
        # Second, use tshark to interpret the fake TCP packets in
        # human-readable form.
        # pylint: disable=consider-using-with
        self.tshark = subprocess.Popen(
            [
                "tshark",
                # Print raw packet data inline with the analysis (aids debugging).
                "-x",
                # Print the detailed human-readable decoding.
                "-V",
                # Input from stdin.
                "--interface",
                "-",
            ],
            stdin=self.text2pcap.stdout,
            stdout=self.stdout,
            stderr=subprocess.DEVNULL,
        )
        self.text2pcap.stdout.close()

    def finalize(self):
        """Call this when done writing the input data into the pipeline."""
        self.text2pcap.stdin.close()

    def getstdout(self):
        """Returns the human-readable output stream. May block."""
        # Ensure the input has been flushed. The caller is supposed to do this
        # after writing all the input, but just to make sure.
        self.finalize()
        # Block until child programs have finished processing.
        self.text2pcap.wait(timeout=10)
        self.tshark.wait(timeout=10)
        # Rewind the output so we can read the data we just wrote.
        self.stdout.seek(0)
        return self.stdout

    def close(self):
        """Close the human-readable output stream."""
        self.stdout.close()


def to_libwayland_timestamp_ms(timestamp_sec):
    """Convert seconds since epoch to a libwayland-compatible timestamp."""
    # libwayland's debugging routine has some integer overflow bugs when
    # printing timestamps. We want our timestamps to interleave with its, so
    # let's replicate those bugs here.
    # TODO(cpelling): Fix libwayland instead.
    #
    # First, convert to microseconds since epoch.
    timestamp_us = timestamp_sec * 1_000_000
    # Truncate to a 32-bit unsigned int.
    timestamp_us %= 1 << 32
    # Convert back to milliseconds.
    timestamp_ms = timestamp_us / 1000
    # Finally, use only 7 digits before the period, emulating
    # libwayland's use of printf.
    timestamp_ms %= 10000000
    return timestamp_ms


class X11Options:
    """Options to control X11 analysis, passed from the command line."""

    def __init__(self):
        self.all_sockets = False
        self.only_new_connections = False
        self.only_known_processes = False
        self.include_frame_data = False


class StraceLine(NamedTuple):
    """Raw parsed data from a single line of strace output."""

    timestamp: float
    syscall: str
    socketinode: int
    peersocketinode: int
    remainder: str

    def data(self):
        """Parse the data packet sent/received in this write/recv call.

        Returns a list of strings. Each string has length 2 and represents
        a single byte from the data packet, in hexadecimal.
        For example: ["00", "ff"]
        """
        # Parse one or more hex strings from 'remainder'; this is our data.
        # In some cases the data is split into multiple buffers; make sure
        # we capture all of them, in order.
        strs = [
            s.strip('"') for s in re.findall(r'"[\\x0-9a-f]+"', self.remainder)
        ]
        return "".join(strs).split(r"\x")[1:]


def parse_strace_line(l):
    """Parse one line of strace output involving send/recv on a Unix socket.

    Returns a structured representation of the data, or None on parse failure.
    """
    # For recv/write calls on Unix sockets, strace outputs:
    # {timestamp} {syscall}({fd}<UNIX>:[{inode}], ...) = {retval}
    #
    # For example (linebreaks inserted for clarity)
    #
    #   1628746867.619794 writev(5<UNIX:[49780]>,
    #       [{iov_base="\x62\x00\x05\x00\x0c\x00\x00\x00\x42
    #       \x49\x47\x2d\x52\x45\x51\x55\x45\x53\x54\x53",
    #       iov_len=20}], 1) = 20
    #
    # If the CONFIG_UNIX_DIAG kernel option is set, strace is able to
    # recover the peer socket's inode and the socket path as well:
    #
    #   1632178278.864598 writev(15<UNIX:[223222->214972,
    #       @"\x2f\x74\x6d\x70\x2f\x2e\x58\x31\x31\x2d\x75\x6e\x69\x78
    #       \x2f\x58\x30"]>,
    #       [{iov_base="\x01\x01\x46\x67\x00\x00\x00\x00\xac\x02\x00
    #       \x00\x29\x00\x00\x02\x8d\x02\xdf\x01\x8d\x02\xdf\x01\x00
    #       \x00\x00\x00\x00\x00\x00\x00", iov_len=32}], 1) = 32
    #
    # This regular expressions extract the timestamp, syscall, socket
    # inode, and peer inode; ignoring the fd and socket path, if any.
    #
    # The socket path is optional. In particular, requiring a socket path
    # will exclude Sommelier's window manager connection to Xwayland.
    #
    # The second argument, containing the hex data, is saved for a
    # second round of parsing later (see StraceLine.data()).
    match = re.match(
        r"([\d.]+) "  # timestamp (capture group 1)
        r"([a-z]+)"  # syscall (capture group 2)
        r"\("  # opening parenthesis for syscall parameters
        r"\d+"  # file descriptor
        r"<(?:UNIX|UNIX-STREAM):"  # socket type
        r"\["  # start socket info
        r"(\d+)"  # socket inode opened by this process (capture group 3)
        r"(?:\->(\d+)|)"  # peer socket's inode (optional capture group 4)
        r',?[\d@"\\xa-f]*'  # optional socket path (do not require; see above)
        r"\]>"  # end socket info
        r", "  # separates syscall parameters
        r"(.*)",  # remainder (capture group 5)
        l,
    )
    if match:
        return StraceLine(
            timestamp=float(match.group(1)),
            syscall=match.group(2),
            socketinode=int(match.group(3)),
            peersocketinode=int(match.group(4)) if match.group(4) else None,
            remainder=match.group(5),
        )
    return None


def analyze_x11_strace_log(
    f, x11_socket_inodes, atoms, x11options, process_names_by_socket_inode
):
    """Analyze the result of running strace on an X server.

    X11 clients typically communicate with the X server using Unix sockets.
    The basic idea is to strace all the X server's recv/write syscalls and
    extract raw X11 protocol bytes received/sent to these sockets. We
    reassemble a fake TCP stream from these packets using text2pcap, feed the
    result to Wireshark, and receive human-readable(ish) descriptions of the
    X11 packets. Finally, we post-process and return one Column for each
    socket.

    This approach has some limitations; for example, Wireshark doesn't fully
    decode all X11 extensions, especially those which can't be used over TCP.
    The main advantages are that we don't need to modify the target system or
    restart any applications to trace them.

    Args:
        f: An open handle to strace's output file.
        x11_socket_inodes: If non-empty, we only consider data sent on sockets
            with these inodes.
        atoms: Dictionary of integer "atom" identifiers to interned strings
            (see an X11 protocol reference). Wireshark only handles built-in
            atoms, so we use this dictionary to decode atoms that are
            allocated at runtime by X clients.
        x11options: See X11Options.
        process_names_by_socket_inode: Map from inodes to a string which lists
            one or more names of processes that have opened that inode.
    """
    # The pcap format enforces a length limit on packets, so we split packets
    # to keep them under this size.
    MAX_PCAP_PACKET_LENGTH = 64000
    columns = []

    analyzers = {}
    matchinglines = 0
    totallines = 0
    with io.TextIOWrapper(f, encoding="ascii") as textfile:
        for l in textfile:
            totallines += 1

            line = parse_strace_line(l)
            if not line:
                continue
            matchinglines += 1

            # Exclude irrelevant syscalls and sockets (by inode).
            if line.syscall not in ("recvfrom", "writev", "recvmsg", "sendmsg"):
                continue
            if x11_socket_inodes and (
                line.socketinode not in x11_socket_inodes
            ):
                continue
            if x11options.only_known_processes and (
                line.peersocketinode not in process_names_by_socket_inode
            ):
                continue

            if line.socketinode not in analyzers:
                if line.peersocketinode:
                    # Attempt to look up which process is communicating
                    # with the X server through this socket.
                    p = process_names_by_socket_inode.get(
                        line.peersocketinode, ""
                    )
                    if p:
                        short_description = p.replace("\n", "")
                        header = f"[{p}]"
                    else:
                        short_description = (
                            f"X client socket {line.peersocketinode} "
                            f"← server socket {line.socketinode}"
                        )
                        header = f"[{short_description}]"
                else:
                    # Fall back to basic info.
                    short_description = (
                        f"X client ← server socket {line.socketinode}"
                    )
                    header = f"[{short_description}]"
                header += (
                    "\n← Event/Reply from X server" "\n→ Request to X server"
                )
                subheader = (
                    "Traffic between client socket "
                    f"{line.peersocketinode}\n"
                    f"and server socket {line.socketinode}, \n"
                    "parsed as X11 packets by Wireshark."
                )
                if line.peersocketinode not in process_names_by_socket_inode:
                    subheader += (
                        "\n\nTip: To exclude this column, pass "
                        "--x11-only-known-processes to trace_window_system.py."
                    )

                analyzers[line.socketinode] = X11AnalysisPipeline(
                    header, short_description, subheader
                )

            data = line.data()
            direction = "I" if line.syscall in ("writev", "sendmsg") else "O"
            for i in range(0, len(data), MAX_PCAP_PACKET_LENGTH):
                packet = " ".join(data[i : i + MAX_PCAP_PACKET_LENGTH])
                # text2pcap is flexible enough to accept 0000 as the "location"
                # field on every line, so we don't need to bother setting it
                # correctly.
                analyzers[line.socketinode].text2pcap.stdin.write(
                    f"{direction} {line.timestamp} 0000 {packet}\n".encode(
                        "ascii"
                    )
                )

    if matchinglines == 0:
        print(
            f"\nWARNING: No X11 packets parsed from {f.name}. "
            f"{matchinglines} of {totallines} matched the regex.\n"
        )

    # Done writing data
    for analyzer in analyzers.values():
        analyzer.finalize()

    # Keeps track of window names.
    windownamesbyid = {}

    def ordering_key(header):
        """Establish a column ordering for X11 clients.

        The order is designed such that processes "closer" to Exo are
        displayed further to the right, and less interesting processes
        are displayed further to the left. This is consistent with the
        placement of Wayland columns to the right of X columns, which
        is not handled here.
        """
        if header.startswith("[X client"):
            # Sockets with unknown process names are leftmost
            return f"aaa{header}"
        if header.startswith("[steam"):
            # Steam is next
            return f"bbb{header}"
        if header.startswith("[sommelier |"):
            # Sommelier is rightmost
            return f"ddd{header}"
        # Other known processes (presumably games) go between Steam and
        # Sommelier.
        return f"ccc{header}"

    # Parse output from each pipeline
    for inode in sorted(
        analyzers.keys(), key=lambda i: ordering_key(analyzers[i].header)
    ):
        column = Column(
            analyzers[inode].header,
            analyzers[inode].short_description,
            analyzers[inode].subheader,
            [],
            valid=not x11options.only_new_connections,
        )

        # It's confusing to use window names across columns, since any modifications won't be
        # processed in order. Keep them within the columns.
        windownamesbyid.clear()

        def appendpacket(timestamp_ms, packet, column):
            if packet:
                column.loglines.append(
                    LogLine(
                        timestamp_ms,
                        wireshark.massage_x11_packet(
                            packet, atoms, windownamesbyid
                        ),
                    )
                )
                if "Initial connection" in packet:
                    column.valid = True

        tsharkout = analyzers[inode].getstdout()
        with io.TextIOWrapper(tsharkout, encoding="utf-8") as tsharktext:
            timestamp_ms = float(-1)
            packet = ""
            for line in tsharktext:
                newpacket = ""
                # Example lines:
                #   Epoch Time: 1667775431.410118
                #   Epoch Arrival Time: 1667775431.410118
                epoch_match = re.match(r"\s+Epoch.* Time: (.+)", line)
                if epoch_match:
                    # The timestamp is listed in the Frame section, before
                    # any X11 sections.
                    timestamp_sec = float(epoch_match.group(1))
                    timestamp_ms = to_libwayland_timestamp_ms(timestamp_sec)
                elif line.startswith("X11, "):
                    # Start a new X11 section
                    newpacket = line

                if x11options.include_frame_data:
                    line_starts_new_packet = line.startswith("Frame ")
                else:
                    line_starts_new_packet = not line.strip()

                if packet and (newpacket or line_starts_new_packet):
                    # Finish previous packet
                    appendpacket(timestamp_ms, packet, column)
                    packet = newpacket
                elif newpacket:
                    # Start a new packet (no previous packet to finish)
                    packet = newpacket
                elif packet:
                    # Keep adding lines to the current packet
                    packet += line

            # Don't forget the final packet
            appendpacket(timestamp_ms, packet, column)

        # Return only non-empty columns which passed the "new connection" check.
        if len(column.loglines) > 0 and column.valid:
            columns.append(column)

    return columns


def analyze_trace(
    filename,
    f,
    pids,
    x11_socket_inodes,
    atoms,
    x11options,
    process_names_by_socket_inode,
):
    """Return a list of Columns produced by analyzing the given file."""

    if "wayland-debug/" in filename:
        return analyze_wayland_trace(filename, f, pids)

    if "x_strace_pid" in filename:
        return analyze_x11_strace_log(
            f,
            x11_socket_inodes,
            atoms,
            x11options,
            process_names_by_socket_inode,
        )

    # The archive contains a number of metadata files which don't
    # themselves produce columns.
    return []


def filename_sort_key(filename):
    """Sort the filenames of the source logs.

    This ensures the columns come out in the desired display order.
    """
    # Sorting in reverse PID order guarantees that client apps
    # go on the left, then Xwayland, then sommelier on the right.
    key = -2 * (extract_pid(filename) or 0)
    if "x11-debug" in filename:
        # X11 output is verbose so X11 apps are leftmost.
        key -= 100000000
    if "client" in filename:
        # Tie-breaker: Clients are to the right of servers.
        key += 1
    return key


def parse_x11_socket_inodes(f):
    """Parse /proc/net/unix to find all the X11 Unix sockets.

    Returns:
        The set of inodes of all X11 Unix sockets found.
    """
    inodes = set()
    if f:
        with io.TextIOWrapper(f, encoding="ascii") as textfile:
            for line in textfile:
                if "/tmp/.X11-unix/X" in line:
                    # The inode is the 7th column
                    inodes.add(int(line.split()[6]))
    return inodes


def parse_lsof(f):
    """Parse lsof's output into a map from socket inodes to process names."""
    if not f:
        return {}
    process_by_inode = {}
    with io.TextIOWrapper(f, encoding="ascii") as textfile:
        pid = -1
        name = "<unknown process>"

        # lsof's machine-readable format is one line per field,
        # Each field is prefixed with a single-letter identifier,
        # starting with 'p' for the process ID.
        for line in textfile:
            if not line:
                continue
            if line[0] == "p":
                # Process ID
                pid = int(line[1:])
                name = "<unknown process>"
            elif line[0] == "c":
                # Process name
                name = line[1:].rstrip()
            elif line[0] == "i":
                # Inode number
                inode = int(line[1:])
                item = f"{name} | PID {pid}"

                # Multiple processes can have the same socket fd (for example,
                # upon fork()). Also, we need to deduplicate multiple entries
                # which will often appear in lsof's output. The natural
                # solution is to make each value a set(). However, Python sets
                # are not ordered, whereas dicts are (since Python 3.7), and
                # it's nice to preserve the original order of the data. So,
                # use a dict to represent a set. The values are ignored and can
                # be anything; I've chosen None.
                process_by_inode.setdefault(inode, {})
                process_by_inode[inode][item] = None

    # Return a flattened dict where values are strings.
    return {k: ",\n ".join(v.keys()) for (k, v) in process_by_inode.items()}


def parse_x11_atoms(f):
    """Parses the output of xlsatoms into a dictionary."""
    atoms = {}
    if f:
        with io.TextIOWrapper(f, encoding="ascii") as textfile:
            for line in textfile:
                atom, name = [x.strip() for x in line.split("\t")]
                atoms[int(atom)] = name
    return atoms


def parse_pids(f):
    """Parse a mapping of Process IDs to command lines from the pids file."""
    pids = {}
    if f:
        with io.TextIOWrapper(f, encoding="ascii") as textfile:
            for line in textfile:
                l = line.split(":", maxsplit=1)
                if len(l) == 2:
                    pids[int(l[0])] = l[1].strip()
    return pids


@contextlib.contextmanager
def extract_matching_file(archive, substring):
    """Open the sole file whose name contains the given substring.

    Returns:
        The sole file object within the given archive matching the given
        substring, or None if there are 0 or 2+ such matching files.
    """
    filenames = [name for name in archive.getnames() if substring in name]
    if len(filenames) == 1:
        with archive.extractfile(filenames[0]) as f:
            yield f
    else:
        print(
            "Warning: Expected 1 file matching %s but found %d:\n\t%s"
            % (substring, len(filenames), "\n\t".join(filenames))
        )
        yield None


def analyze_raw_log(filename):
    """Return a list of columns displaying a simple log file."""
    with open(filename, "r", encoding="utf-8") as f:
        lines = []
        failures = 0
        for line in f:
            line = line.rstrip("\n")

            # Handle continuation lines printed by `strace --stack-traces`;
            # fold them into the previous line.
            if line.startswith(" > ") and len(lines) > 0:
                lines[-1].text += "\n" + line
                continue

            # Otherwise, attempt to split a timestamp from the front.
            segments = line.split(maxsplit=1)
            if len(segments) == 2:
                try:
                    timestamp = to_libwayland_timestamp_ms(float(segments[0]))
                    lines.append(LogLine(timestamp, segments[1]))
                except ValueError:
                    failures += 1
            else:
                failures += 1

        if failures > 0:
            total = failures + len(lines)
            print(
                f"WARNING: {failures} of {total} lines of {filename} failed to parse"
            )

        if len(lines) > 0:
            col = Column(filename, "", "", [])
            col.loglines.extend(lines)
            return [col]
    return []


def analyze_archive_file(filename, output_filename, with_logs, x11options):
    """Analyze a tar archive produced by trace_window_system.sh.

    Args:
        filename: Path to the tar archive.
        output_filename: Path to output file.
        with_logs: List of filenames of additional timestamped logs to include in
            the output.
        x11options: See X11Options.
    """
    columns = []

    # Add "extra" logs first; as they are presumably of interest,
    # displaying them in the leftmost columns makes sense.
    for log in with_logs:
        columns += analyze_raw_log(log)

    # Data for the window rect visualizer.
    visualizer_frames = []

    with tarfile.open(filename) as archive:
        # Map process IDs to each process's command line.
        pids = {}
        with extract_matching_file(archive, "pids") as f:
            pids = parse_pids(f)

        # List all atoms
        atoms = {}
        with extract_matching_file(archive, "xlsatoms.log") as f:
            atoms = parse_x11_atoms(f)

        # List all the X11 socket inodes.
        x11_socket_inodes = None
        if not x11options.all_sockets:
            with extract_matching_file(archive, "proc_net_unix") as f:
                x11_socket_inodes = parse_x11_socket_inodes(f)

        # Associate socket inodes with processes.
        process_names_by_socket_inode = {}
        with extract_matching_file(archive, "lsof.log") as f:
            process_names_by_socket_inode = parse_lsof(f)

        # Build lists of X window positions/sizes at the beginning and end of the trace.
        with extract_matching_file(
            archive, "xwindump_before.log"
        ) as binaryfile:
            if binaryfile:
                with io.TextIOWrapper(binaryfile, encoding="utf-8") as f:
                    visualizer_frames.append(
                        visualizer.xwindump_to_frame(
                            f.read(), 0, "Initial state"
                        )
                    )
        with extract_matching_file(archive, "xwindump_after.log") as binaryfile:
            if binaryfile:
                with io.TextIOWrapper(binaryfile, encoding="utf-8") as f:
                    visualizer_frames.append(
                        visualizer.xwindump_to_frame(f.read(), 0, "Final state")
                    )

        # Analyze remaining archive files.
        for memberfilename in sorted(archive.getnames(), key=filename_sort_key):
            with archive.extractfile(memberfilename) as memberfile:
                columns += analyze_trace(
                    memberfilename,
                    memberfile,
                    pids,
                    x11_socket_inodes,
                    atoms,
                    x11options,
                    process_names_by_socket_inode,
                )

    render(columns, visualizer_frames, output_filename)


def analyze_x11_strace_file(
    filename, out, with_logs, x11options, process_names_by_socket_inode
):
    """Analyze strace's output, assuming it was run on an X server.

    Attempts to recover X11 packets sent/received on Unix sockets by the
    traced process. Since we have no way to distinguish X11 sockets from other
    sockets using this method, we attempt to parse X11 data from all sockets.
    This will produce extra columns containing junk output.
    """

    columns = []

    # Add "extra" logs first; as they are presumably of interest,
    # displaying them in the leftmost columns makes sense.
    for log in with_logs:
        columns += analyze_raw_log(log)

    with open(filename, "rb") as f:
        columns += analyze_x11_strace_log(
            f, None, {}, x11options, process_names_by_socket_inode
        )

    render(columns, [], out)


def main():
    """main program routine."""

    parser = argparse.ArgumentParser(description=__doc__)

    args_to_forward_to_container = [
        "type",
        "x11_only_new_connections",
        "x11_only_known_processes",
        "x11_all_sockets",
        "x11_include_frame_data",
    ]

    parser.add_argument("--file", required=True, help="Input file to analyze.")
    parser.add_argument(
        "--with-log",
        action="append",
        help="Add a column for a timestamped log file (for example, "
        "from xtrace). May be repeated.",
    )
    parser.add_argument(
        "--out",
        help="Write analysis to the given file. If not specified, uses a"
        "filename generated based on --file.",
    )
    parser.add_argument(
        "--type",
        choices=["archive", "x11-strace"],
        default="archive",
        help="archive (the default) analyzes a full .tar.gz archive; "
        "x11-strace analyzes only an strace of a process that makes X11 "
        "requests; this is only really useful for debugging this script.",
    )
    parser.add_argument(
        "--x11-only-new-connections",
        action="store_true",
        help="Omit X11 sockets that were opened before strace attached. "
        "Useful for filtering down to only new clients. Also, the X11 data "
        "is easier to read if Wireshark sees the entire connection, since it "
        "can discover which extensions are supported by the server, decode "
        "atoms, and so on.",
    )
    parser.add_argument(
        "--x11-only-known-processes",
        action="store_true",
        help="Omit X11 sockets whose owning process is not known. For "
        "example, these might be short-lived connections which tend to be less "
        "interesting.",
    )
    parser.add_argument(
        "--x11-all-sockets",
        action="store_true",
        help="Assume all strace'd sockets are speaking X11. Mostly only"
        "useful for debugging this script.",
    )
    parser.add_argument(
        "--x11-include-frame-data",
        action="store_true",
        help="Append a hexdump of the raw packet data to decoded X11 packets. "
        "For advanced usage. NB: the hexdump includes fake TCP headers.",
    )
    # TODO(davidriley): Use action=argparse.BooleanOptionalAction when chroot has Python 3.9.
    parser.add_argument(
        "--container",
        action="store_true",
        default=True,
        help="Relaunch this script inside a Docker container, to ensure all "
        "required dependencies are installed at a consistent version.",
    )
    parser.add_argument(
        "--no-container",
        dest="container",
        action="store_false",
        help="Run this script directly, without creating a container.",
    )

    args = parser.parse_args()

    if args.out:
        out_filename = args.out
    else:
        # Strip all file extensions.
        # Call splitext twice because we expect a .tar.gz extension on the
        # input file, and splitext only removes one segment at a time.
        stripped = os.path.splitext(os.path.splitext(args.file)[0])[0]
        out_filename = f"{stripped}.csv"

    if args.container:
        tools_dir = os.path.dirname(sys.argv[0])
        build_dir = os.path.join(tools_dir, "..", "build")

        # Build and tag the Docker image we'll use to run the script.
        try:
            docker_build = [
                "sudo",
                "env",
                "DOCKER_BUILDKIT=1",
                "docker",
                "build",
                "-f",
                os.path.join(build_dir, "tools.Dockerfile"),
                "--tag=trace-window-system",
                "--target=trace-window-system",
                build_dir,
            ]
            print("Running", docker_build)
            subprocess.check_call(docker_build)
        except:
            print(
                "trace_window_system.py: Couldn't run docker. Ensure Docker is installed, see "
                "preceding logs, or pass --no-container to run without Docker."
            )
            raise

        # Set up to mount necessary files into the container.
        input_dirs = [
            (tools_dir, "/tools"),
            (os.path.dirname(args.file), "/windowtraceinput"),
        ]
        output_dirs = [
            (os.path.dirname(out_filename), "/windowtraceoutput"),
        ]

        file_in_container = os.path.join(
            "/windowtraceinput", os.path.basename(args.file)
        )
        out_file_in_container = os.path.join(
            "/windowtraceoutput", os.path.basename(out_filename)
        )

        # Build the argument list for the containerized script.
        forwarded_args = [
            "--file",
            file_in_container,
            "--out",
            out_file_in_container,
        ]

        for i, arg in enumerate(args.with_log or []):
            inside_dir = f"/windowtrace_withlog{i}"
            outside_dir = os.path.dirname(arg)
            input_dirs.append((outside_dir, inside_dir))

            forwarded_args.append("--with-log")
            forwarded_args.append(
                os.path.join(inside_dir, os.path.basename(arg))
            )

        for a in args_to_forward_to_container:
            assert a in vars(args)
            val = vars(args)[a]
            cmdline_arg = "--" + a.replace("_", "-")
            if val is True:
                forwarded_args.append(cmdline_arg)
            elif val is False:
                pass
            else:
                forwarded_args.append(cmdline_arg)
                forwarded_args.append(str(val))

        # Run the container, with required mounts and arguments.
        docker_run = [
            "sudo",
            "docker",
            "run",
            # This is required to launch dumpcap without EPERM.
            "--cap-add=NET_ADMIN",
        ]
        for source, target in input_dirs:
            docker_run.append("--mount")
            docker_run.append(
                f"type=bind,source={source},target={target},readonly"
            )
        for source, target in output_dirs:
            docker_run.append("--mount")
            docker_run.append(f"type=bind,source={source},target={target}")
        docker_run.append("trace-window-system")
        docker_run += forwarded_args
        print("Running", docker_run)
        subprocess.check_call(docker_run)

        # All done!
        sys.exit(0)

    x11options = X11Options()
    x11options.all_sockets = args.x11_all_sockets
    x11options.only_new_connections = args.x11_only_new_connections
    x11options.only_known_processes = args.x11_only_known_processes
    x11options.include_frame_data = args.x11_include_frame_data

    with_logs = args.with_log or []
    if args.type == "archive":
        analyze_archive_file(args.file, out_filename, with_logs, x11options)
    elif args.type == "x11-strace":
        analyze_x11_strace_file(
            args.file, out_filename, with_logs, x11options, {}
        )


if __name__ == "__main__":
    main()  # pylint: disable=no-value-for-parameter


class TestParseStraceLine(unittest.TestCase):
    """Tests for parse_strace_line()."""

    def test_without_config_unix_diag(self):
        """Parses strace format when kernel doesn't have CONFIG_UNIX_DIAG."""
        line = (
            r"""1628746867.619794 writev(5<UNIX:[49780]>, """
            r"""[{iov_base="\x62\x00\x05\x00\x52\x45\x51\x55","""
            r"""iov_len=20}], 1) = 20"""
        )
        output = parse_strace_line(line)
        self.assertEqual(output.timestamp, 1628746867.619794)
        self.assertEqual(output.syscall, "writev")
        self.assertEqual(output.socketinode, 49780)
        self.assertEqual(
            output.data(), ["62", "00", "05", "00", "52", "45", "51", "55"]
        )

    def test_with_config_unix_diag(self):
        """Parses strace format when kernel does have CONFIG_UNIX_DIAG."""
        line = (
            r"""1667775430.980439 recvmsg("""
            r"""39<UNIX-STREAM:[12461->21714,"""
            r"""@"\x2f\x74\x6d\x70\x2f"""
            r"""\x2e\x58\x31\x31\x2d\x75\x6e\x69\x78\x2f\x58\x30"]>, """
            r"""{msg_name=NULL, msg_namelen=0, msg_iov=[{"""
            r"""iov_base="\x6c\x00\x0b\x00", iov_len=16380"""
            r"""}], msg_iovlen=1, msg_controllen=0, msg_flags=0}, 0) = 12"""
        )
        output = parse_strace_line(line)
        self.assertEqual(output.timestamp, 1667775430.980439)
        self.assertEqual(output.syscall, "recvmsg")
        self.assertEqual(output.socketinode, 12461)
        self.assertEqual(output.peersocketinode, 21714)
        self.assertEqual(output.data(), ["6c", "00", "0b", "00"])

    def test_with_config_unix_diag_unnamed_socket(self):
        """Parses strace format for unnamed sockets with CONFIG_UNIX_DIAG.

        Sommelier is the typical (only?) example of this case.
        """
        line = (
            r"""1667775431.410118 recvmsg("""
            r"""30<UNIX-STREAM:[13358->13357]>, """
            r"""{"""
            r"""msg_name=NULL, msg_namelen=0, msg_iov=["""
            r"""{iov_base="\x11\x00\x00\x00", iov_len=3480}, """
            r"""{iov_base="", iov_len=616}], """
            r"""msg_iovlen=2, msg_controllen=0, msg_flags=MSG_CMSG_CLOEXEC"""
            r"""}, """
            r"""MSG_DONTWAIT|MSG_CMSG_CLOEXEC"""
            r""") = 4"""
        )
        output = parse_strace_line(line)
        self.assertEqual(output.timestamp, 1667775431.410118)
        self.assertEqual(output.syscall, "recvmsg")
        self.assertEqual(output.socketinode, 13358)
        self.assertEqual(output.peersocketinode, 13357)
        self.assertEqual(len(output.data()), 4)


class TestParseLsof(unittest.TestCase):
    """Tests for parse_lsof()."""

    def test_returns_empty(self):
        """parse_lsof() returns empty output for empty input."""
        with io.BytesIO() as f:
            self.assertEqual(parse_lsof(f), {})

    def test_parses_one_item(self):
        """parse_lsof() parses one process with one inode."""

        lsof = b"p5\n" b"cXorg\n" b"i123\n"
        output = {123: "Xorg | PID 5"}

        with io.BytesIO(lsof) as f:
            self.assertEqual(parse_lsof(f), output)

    def test_parses_multiple_items(self):
        """parse_lsof() parses multiple processes with multiple inodes each."""

        lsof = (
            b"p5\n"
            b"cXorg\n"
            b"i123\n"
            b"i234\n"
            b"i56789\n"
            b"p273\n"
            b"csommelier\n"
            b"i1000\n"
            b"i1001\n"
            b"i2000\n"
        )
        output = {
            123: "Xorg | PID 5",
            234: "Xorg | PID 5",
            56789: "Xorg | PID 5",
            1000: "sommelier | PID 273",
            1001: "sommelier | PID 273",
            2000: "sommelier | PID 273",
        }

        with io.BytesIO(lsof) as f:
            self.assertEqual(parse_lsof(f), output)

    def test_parses_overlapping_inodes(self):
        """parse_lsof() handles inodes shared between processes."""
        lsof = (
            b"p100\n"
            b"cPARENT\n"
            b"i1000\n"
            b"i2000\n"
            b"p200\n"
            b"cCHILD\n"
            b"i1000\n"
            b"i1500\n"
        )
        output = {
            1000: "PARENT | PID 100,\n CHILD | PID 200",
            2000: "PARENT | PID 100",
            1500: "CHILD | PID 200",
        }

        with io.BytesIO(lsof) as f:
            self.assertEqual(parse_lsof(f), output)

    def test_deduplicates(self):
        """parse_lsof() de-duplicates multiple identical entries."""
        lsof = (
            b"p5\n"
            b"cmyapp\n"
            b"i123\n"
            b"p5\n"
            b"cmyapp\n"
            b"i123\n"
            b"p5\n"
            b"cmyapp\n"
            b"i123\n"
        )
        output = {123: "myapp | PID 5"}

        with io.BytesIO(lsof) as f:
            self.assertEqual(parse_lsof(f), output)

    def test_handles_missing_names(self):
        """parse_lsof() handles unnamed processes."""
        lsof = b"p1000\n" b"i123456789\n"
        output = {123456789: "<unknown process> | PID 1000"}

        with io.BytesIO(lsof) as f:
            self.assertEqual(parse_lsof(f), output)
