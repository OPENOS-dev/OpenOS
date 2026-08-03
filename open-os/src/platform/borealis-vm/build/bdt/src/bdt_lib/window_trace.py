# Copyright 2022 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Collect a set of trace data for tools/trace_window_system.py to analyze.

 Specifically, collects libwayland debug output, an strace of the X server,
 and various related system state to aid interpretation. Collected logs are
 combined into a gzipped tar archive as expected by trace_window_system.py.
"""

import glob
import io
import os
import re
import shutil
import signal
import subprocess
import tempfile
import time


def file_ends_with(f, suffix):
    """Whether the file |f| currently ends with |suffix|.

    Args:
        f: A file, opened in binary mode (to support seeking-from-end).
        suffix: The suffix to test for, as a bytes object
    """
    try:
        f.seek(-len(suffix), io.SEEK_END)
    except OSError:
        return False
    return f.read() == suffix


def toggle_wayland_debug(enabled):
    """Enable/disable Wayland debug logs.

    Args:
        enabled: True to enable, False to disable.
    """
    # TODO(cpelling): Migrate toggle-wayland-debug to be a Python function
    # instead of an external program.
    try:
        subprocess.run(
            ["toggle-wayland-debug", "--enable" if enabled else "--disable"],
            check=True,
        )
    except FileNotFoundError:
        print(
            "WARNING: toggle-wayland-debug not installed. "
            "Wayland logs will be missing."
        )


def collect_xwindump(xwindump_out):
    """Run xwindump.py, saving stdout to the given filename."""
    with open(xwindump_out, "wb") as f:
        xwindump = subprocess.run(
            ["/usr/bin/xwindump.py"], check=False, stdout=f
        )
        if xwindump.returncode:
            print(
                f"Warning: xwindump.py exited with code {xwindump.returncode}. Skipping."
            )


def collect_window_trace(xpid, archive_out, tempdir):
    """Collect a trace for analysis by tools/trace_window_system.py.

    Args:
        xpid: Process ID of the X server to trace.
        archive_out: Output filename.
        tempdir: A temporary directory to write intermediate files to.
    """
    lsof_out = os.path.join(tempdir, "lsof.log")
    pids_out = os.path.join(tempdir, "pids.log")
    strace_out = os.path.join(tempdir, f"x_strace_pid{xpid}.log")
    xlsatoms_out = os.path.join(tempdir, "xlsatoms.log")
    xwindump_before_out = os.path.join(tempdir, "xwindump_before.log")
    xwindump_after_out = os.path.join(tempdir, "xwindump_after.log")

    # Later we'll gather up all the new logs from this directory. Remember
    # which logs were already there, so we can avoid collecting the old ones.
    old_wayland_logs = set(glob.glob("/tmp/wayland-debug/*.log"))

    # Save the X11 window state at the beginning.
    collect_xwindump(xwindump_before_out)

    strace_args = [
        "strace",
        # System timestamps in seconds since epoch.
        "-ttt",
        # Omit status and exit messages.
        "-qq",
        # All strings in hex (easier to convert to binary data later).
        "-xx",
        # Trace syscalls which are used to read/write on Unix sockets.
        "-e",
        "writev,recvmsg,recvfrom,sendmsg",
        # Map socket file descriptors to inodes. We can later recover process
        # IDs/names from the inode numbers using lsof's output (see below).
        "-yy",
        # Don't truncate long strings (so we can recover whole packets).
        "-s",
        "9999999",
        # Trace the X server process.
        f"--attach={xpid}",
        # Write to designated temporary file.
        f"--output={strace_out}",
    ]

    # Generate information for mapping inodes to processes.
    lsof_args = [
        "lsof",
        # Print command lines and inodes, in parseable form (-F)
        "-Fci",
        # Print numbers instead of network names.
        "-n",
        # Print numbers instead of port names.
        "-P",
        # Print user ID numbers instead of login names.
        "-l",
        # Select Unix socket domain files for printing.
        "-U",
        # "And" multiple args.
        "-a",
        #  Repeat every 10 seconds until killed.
        "-r10",
    ]

    # Run tracing utilities in parallel until interrupted by Ctrl-C / SIGINT.
    with subprocess.Popen(strace_args) as strace, open(
        lsof_out, "w", encoding="utf-8"
    ) as lsof_out_f, subprocess.Popen(lsof_args, stdout=lsof_out_f) as lsof:
        toggle_wayland_debug(True)
        # TODO(cpelling): capture /proc/net/unix repeatedly
        try:
            print("Tracing until interrupted, press Ctrl-C to stop.")
            signal.sigwait([signal.SIGINT])
        finally:
            lsof.kill()
            strace.kill()
            toggle_wayland_debug(False)

    # Invoking toggle-wayland-debug causes libwayland to log to files in
    # /tmp/wayland-debug, so gather any new such files.
    #
    # See https://crrev.com/c/3008027. Borealis incorporates this patch by
    # default. On other platforms, you'll need to manually build and
    # install libwayland with that patch.
    wayland_logs = set(glob.glob("/tmp/wayland-debug/*.log"))
    wayland_logs.difference_update(old_wayland_logs)

    # Bundle up any extra logs produced by manual runs of strace/xtrace/etc.
    # The trailing wildcard is for compatibility with `strace -ff`, which
    # creates one file per process with a trailing '.<pid>' extension.
    extra_logs = glob.glob("/tmp/trace_*.log*")

    # Log PIDs and cmdlines of relevant processes.
    # This includes the X server we ran strace on, and any process which wrote
    # a libwayland log. In the latter case, extract the PID from Wayland log
    # filenames, which are similar to
    # "/tmp/wayland-debug/client-pid264-0x5ccf8b815c88.log".
    pids = set([xpid])
    for log in wayland_logs:
        match = re.search("pid([0-9]+)", log)
        if match and match.group(1).isdigit():
            pids.add(int(match.group(1)))
    with open(pids_out, "w", encoding="utf-8") as pids_out_f:
        for pid in pids:
            try:
                # TODO(cpelling): This should be done while the trace is
                # running, to avoid missing processes that exit before the
                # trace ends.
                with open(
                    f"/proc/{pid}/cmdline", "r", encoding="utf-8"
                ) as cmdline:
                    cmd = cmdline.read().replace("\x00", "\t")
                    pids_out_f.write(f"{pid}:{cmd}\n")
            except FileNotFoundError:
                # Skip this if the process has exited.
                pass

    # The X11 protocol uses ints to stand in for strings in many cases.
    # These int/string pairs are called atoms. They vary between runs and
    # are often important for understanding the semantics of various protocol
    # messages, so save a list of them.
    with open(xlsatoms_out, "w", encoding="utf-8") as f:
        subprocess.run(
            ["xlsatoms", "-display", os.environ.get("DISPLAY", ":0")],
            check=True,
            stdout=f,
        )

    # Save the X11 window state at the end.
    collect_xwindump(xwindump_after_out)

    # Poll for Wayland logs to finish writing. Otherwise this process can race
    # with libwayland-using processes that haven't closed their log files yet.
    #
    # The closing signal sent by toggle_wayland_debug may not be received
    # until the Wayland event loop attempts to print a debug line, so in
    # practice this may not terminate until user input occurs.
    print("Finalizing Wayland logs:")
    for log in wayland_logs:
        print(f"\t{log}...")
        with open(log, "rb") as f:
            while not file_ends_with(f, b"+++LOG END+++\n"):
                time.sleep(1)
    print("...done")

    subprocess.run(
        [
            "tar",
            "--create",
            "--file",
            archive_out,
            "--gzip",
            "--verbose",
            pids_out,
            lsof_out,
            # TODO: proc_net_unix_out,
            xlsatoms_out,
            strace_out,
            xwindump_before_out,
            xwindump_after_out,
        ]
        + list(wayland_logs)
        + extra_logs,
        check=True,
    )

    # On success, remove all archived Wayland logs.
    for log in wayland_logs:
        os.remove(log)

    return True


def collect_window_trace_command(xserver, out):
    """Command-line interface for collect_window_trace().

    Args:
        xserver: Process name or process ID of the X11 server to trace.
        out: Output filename.
    """
    # Find the X server's process ID
    if xserver.isdecimal():
        xpid = int(xserver)
    else:
        pidof = subprocess.run(
            ["pidof", "-s", xserver],
            capture_output=True,
            encoding="utf8",
            check=True,
        )
        pid = pidof.stdout.strip()
        if pidof.returncode == 0 and pid.isdecimal():
            xpid = int(pid)
        else:
            print(
                f"ERROR: Could not find X server process ({xserver}); "
                f'pidof returned {pidof.returncode} with output "{pid}"'
            )
            return

    tempdir = tempfile.mkdtemp(prefix="windowtrace")
    try:
        collect_window_trace(xpid, out, tempdir)

        shutil.rmtree(tempdir)

        print(
            f"Success: Created {out}\n"
            "If running a test image, download with:\n"
            f"  ssh dut -- borealis-sh cat {out} > {out}"
        )
    except:
        # On failure, keep the partial artifacts around in case the dev still
        # wants them.
        print(
            "\nWarning: window-trace collection failed.\n"
            f"         Temporary directory {tempdir} not cleaned up.\n"
        )
        raise
