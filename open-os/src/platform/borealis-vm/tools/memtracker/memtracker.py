#!/usr/bin/env python3
# Copyright 2023 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Summarize memory state of both host and guest every second.

This script runs two copies of `detailed_metrics --quick-stats`: one
on the ChromeOS host, and one on the Borealis guest.  It combines
their output and writes it to `/tmp/combined_metrics.json` on the
guest, and once a second, outputs a guess at the overall state of
system memory.
"""

import argparse
import contextlib
import json
import os
import select
import subprocess
import sys
import time


def check_call(s):
    """Print out a command, then run it."""
    print(s)
    return subprocess.check_call(s, shell=True)


def check_output(s):
    """Print out a command, then run it, returning its output."""
    print(s)
    return subprocess.check_output(s, shell=True).decode()


class JsonCollector:
    """Read and parse JSON objects returned by detailed_metrics."""

    def __init__(self, source, fileobj):
        self.source = source
        self.fileobj = fileobj
        self.input_buffer = ""

    def collect(self):
        """Read available input, and return collected JSON objects."""
        data = self.input_buffer + self.fileobj.read().decode()
        parts = data.split("\n")
        # Stash any partially received data for next time.
        self.input_buffer = parts.pop()
        # Decode any fully received lines, if any.
        reports = []
        for part in parts:
            report = json.loads(part)
            # Tag report with data source.
            report["source"] = self.source
            # This isn't 100% accurate, but close enough.
            report["timestamp"] = time.time()
            reports.append(report)
        return reports


def fmt_amount(amount):
    """Convert a size in KiB to a human readable string."""
    for suffix in "KM":
        if amount < 1024:
            return f"{amount:.1f} {suffix}i"
        amount /= 1024
    return f"{amount:.2f} Gi"


class Analyzer:
    """Produce simple memory stats from detailed_metrics reports."""

    def __init__(self, host_only):
        self.reports = {}
        self.last_output_time = None

        if host_only:
            # Insert a fake guest report to allow host calculations to work.
            self.handle_report(
                {
                    "source": "guest",
                    "MemTotal": 0,
                    "MemAvailable": 0,
                    "MemFree": 0,
                    "balloon_inflate": 0,
                    "balloon_deflate": 0,
                    "Buffers": 0,
                    "Cached": 0,
                    "SwapTotal": 0,
                    "SwapFree": 0,
                    "Unevictable": 0,
                }
            )

    def sources(self):
        """Return list of sources seen, to detect host-only recordings."""
        return list(self.reports.keys())

    def handle_report(self, report):
        """Accept a single report, and maybe output a status line."""
        self.reports[report["source"]] = report
        if "host" not in self.reports or "guest" not in self.reports:
            # Not ready yet.
            return None
        now = report["timestamp"]
        if not self.last_output_time or now - self.last_output_time > 1:
            # It's been a second since the last output; say something.
            self.last_output_time = now
            host = self.reports["host"]
            guest = self.reports["guest"]
            # balloon_inflate and balloon_deflate are counters for inflate/deflate
            # events, which respectively increase or decrease the balloon size by
            # one 4 KiB page.  balloon_inflate - balloon_deflate is the current
            # number of pages in the balloon.
            balloon_size = 4 * (
                guest["balloon_inflate"] - guest["balloon_deflate"]
            )
            # A guess at memory used by the host.
            # Bug: This can go negative when a VM has just started and many
            # of its pages are still unmapped.
            host_used = (
                host["MemTotal"]
                - host["MemAvailable"]
                - guest["MemTotal"]
                + balloon_size
            )
            # A more accurate guess at memory used on the guest.
            guest_used = (
                guest["MemTotal"] - balloon_size - guest["MemAvailable"]
            )
            # This is what `top` reports as used when run on the host.
            host_used_top = (
                host["MemTotal"]
                - host["MemFree"]
                - host["Buffers"]
                - host["Cached"]
            )
            # This is what `top` reports as used (+ balloon) when run on the guest.
            guest_used_top = (
                guest["MemTotal"]
                - guest["MemFree"]
                - guest["Buffers"]
                - guest["Cached"]
                - balloon_size
            )
            derived_info = {
                "timestamp": now,
                "total": host["MemTotal"],
                "total_used": host_used + guest_used,
                "host_used": host_used,
                "guest_used": guest_used,
                "total_avail": host["MemAvailable"] + guest["MemAvailable"],
                "host_avail": host["MemAvailable"],
                "guest_avail": guest["MemAvailable"],
                "balloon": balloon_size,
                "host_swap": host["SwapTotal"] - host["SwapFree"],
                "guest_swap": guest["SwapTotal"] - guest["SwapFree"],
                "host_unevictable": host["Unevictable"],
                "guest_unevictable": guest["Unevictable"],
                "total_gpu": host["i915_gem_objects bytes"] / 1024,
            }
            derived_info["total_incl_swap"] = (
                derived_info["total_used"]
                + derived_info["host_swap"]
                + derived_info["guest_swap"]
            )
            # Disable fstring lint check to maintain readability
            # pylint: disable=consider-using-f-string
            print(
                "total %s, "
                "used %s (top: %s) (host %s (top: %s) guest %s (top: %s)), "
                "avail %s (host %s guest %s), "
                "balloon %s, "
                "swap host %s guest %s, "
                "unevictable host %s guest %s, "
                "total gpu %s"
                % (
                    # Total memory.
                    fmt_amount(derived_info["total"]),
                    # System used memory.
                    fmt_amount(derived_info["total_used"]),
                    fmt_amount(host_used_top + guest_used_top),
                    # Host used memory.
                    fmt_amount(derived_info["host_used"]),
                    fmt_amount(host_used_top),
                    # Guest used memory.
                    fmt_amount(derived_info["guest_used"]),
                    fmt_amount(guest_used_top),
                    # A guess at available memory on the system.
                    fmt_amount(derived_info["total_avail"]),
                    fmt_amount(derived_info["host_avail"]),
                    fmt_amount(derived_info["guest_avail"]),
                    # Virtio balloon size.
                    fmt_amount(derived_info["balloon"]),
                    # Swap usage.
                    fmt_amount(derived_info["host_swap"]),
                    fmt_amount(derived_info["guest_swap"]),
                    # Unevictable memory.
                    fmt_amount(derived_info["host_unevictable"]),
                    fmt_amount(derived_info["guest_unevictable"]),
                    # GPU memory.
                    fmt_amount(derived_info["total_gpu"]),
                )
            )
            # pylint: enable=consider-using-f-string
            return derived_info
        return None


class CombinedTracker:
    """Report combined info on the host and guest together."""

    def __init__(
        self, analyzer, run_seconds=0, host_only=False, host_log_path=None
    ):
        self.analyzer = analyzer
        self.host_log_path = host_log_path
        self.host_only = host_only
        self.log_files = []
        self.run_seconds = run_seconds

    def received_report(self, report):
        """Called when we receive a report from a detailed_metrics process."""

        # Dump report to log.
        report_serialized = json.dumps(report) + "\n"
        for logfile in self.log_files:
            logfile.write(report_serialized)
            logfile.flush()

        # Summarize the interesting parts.
        self.analyzer.handle_report(report)

    def main(self):
        """Tracker entry point: collect metrics on host and guest."""

        if self.host_only:
            print("Tracking memory usage in host")
        else:
            print("Tracking memory usage in host + guest")
            try:
                lsb_release = check_output("borealis-sh cat /etc/lsb-release")
            except subprocess.CalledProcessError:
                print(
                    "Guest is inaccessible or not running; start Steam or try --host-only."
                )
                return 1
            print(f"lsb_release = {repr(lsb_release)}")
            if "BOREALIS_STAGE=runtime_prod" in lsb_release:
                print(
                    "Warning: Running on prod Borealis image; "
                    "detailed_metrics will probably not be available."
                )
            # Copy detailed_metrics binary to host
            check_call(
                "borealis-sh -- cat /usr/bin/detailed_metrics > /usr/local/bin/detailed_metrics"
            )
            check_call("chmod +x /usr/local/bin/detailed_metrics")

        print("Starting host collector process")
        with contextlib.ExitStack() as stack:
            print(f"Logging to {self.host_log_path} on the host.")
            host_log = stack.enter_context(
                open(self.host_log_path, "wt", encoding="utf-8")
            )
            host_proc = stack.enter_context(
                subprocess.Popen(
                    "/usr/local/bin/detailed_metrics --quick-stats",
                    shell=True,
                    stdout=subprocess.PIPE,
                )
            )
            if self.host_only:
                return self.combine_metrics(host_proc, None, [host_log])
            print("Starting guest collector process")
            guest_proc = stack.enter_context(
                subprocess.Popen(
                    "borealis-sh -- /usr/bin/detailed_metrics --quick-stats",
                    shell=True,
                    stdout=subprocess.PIPE,
                )
            )
            print("Collector processes started")
            log_proc = stack.enter_context(
                subprocess.Popen(
                    # Use dd with bs=1 to ensure whole lines are written at once.
                    [
                        "/usr/bin/borealis-sh",
                        "--",
                        "dd",
                        "bs=1",
                        "of=/tmp/combined_metrics.json",
                    ],
                    stdin=subprocess.PIPE,
                    # Line-buffered text mode.
                    bufsize=1,
                    universal_newlines=True,
                )
            )
            return self.combine_metrics(
                host_proc, guest_proc, [log_proc.stdin, host_log]
            )
        return 0

    def combine_metrics(self, host_proc, guest_proc, log_files):
        """Read metrics output from host and guest forever, and output to log."""
        print("detailed_metrics started on host and guest.")
        os.set_blocking(host_proc.stdout.fileno(), False)
        poller = select.epoll()
        poller.register(host_proc.stdout.fileno(), select.POLLIN)
        host_collector = JsonCollector("host", host_proc.stdout)
        if not self.host_only:
            os.set_blocking(guest_proc.stdout.fileno(), False)
            poller.register(guest_proc.stdout.fileno(), select.POLLIN)
            guest_collector = JsonCollector("guest", guest_proc.stdout)
        self.log_files = log_files
        # Run for `self.run_seconds` seconds if specified, else forever.
        exit_after = (
            time.time() + self.run_seconds if self.run_seconds else None
        )
        while 1:
            if exit_after and time.time() > exit_after:
                break
            # This should *usually* result in a pair of results coming in around the same time,
            # although I hear that the guest timer can drift, so I wouldn't be surprised if
            # they no longer show up in lockstep.
            for fd, _ in poller.poll(1):
                if fd == host_proc.stdout.fileno():
                    for host_report in host_collector.collect():
                        # print("HOST %s" % repr(host_report))
                        self.received_report(host_report)
                elif guest_proc and fd == guest_proc.stdout.fileno():
                    for guest_report in guest_collector.collect():
                        # print("GUEST %s" % repr(guest_report))
                        self.received_report(guest_report)
                else:
                    print("output on unknown fd -- huh???")

        return 0


def generate_html_from_summary(summary, f):
    """Write HTML to file `f` that displays a graph of the summary data."""

    # Convert summary entries into chart.js format.
    data = {}
    for entry in summary:
        ts = int(entry["timestamp"] * 1000)
        for k, v in entry.items():
            if k == "timestamp":
                continue
            data.setdefault(k, []).append({"x": ts, "y": v})

    # chart.js configuration.
    chart = {
        "type": "line",
        "parsing": False,
        "data": {
            "datasets": [
                {
                    "label": label,
                    "data": values,
                }
                for label, values in data.items()
            ]
        },
        "options": {
            "scales": {
                "x": {
                    "type": "time",
                    "time": {
                        "unit": "second",
                    },
                },
            },
        },
    }

    f.write(
        f"""
    <html>
        <head>
            <title>Borealis memory recording</title>
        </head>
        <body>
            <div>
                <canvas id="myChart"></canvas>
            </div>

            <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
            <script src="https://cdn.jsdelivr.net/npm/chartjs-adapter-date-fns"></script>

            <script>
                const ctx = document.getElementById('myChart');
                new Chart(ctx, {json.dumps(chart)});
            </script>
        </body>
    </html>
    """
    )


def main():
    """Main entry point."""
    parser = argparse.ArgumentParser(description=__doc__)

    parser.add_argument(
        "filename",
        help="Path to combined_metrics.json file to analyze",
        nargs="?",
    )
    parser.add_argument(
        "--host-only",
        help="Run only on host, not Borealis guest",
        action="store_true",
    )
    parser.add_argument(
        "--host-log-path",
        help="Path to combined metrics logfile on host",
        default=os.path.join(
            os.path.dirname(os.path.realpath(__file__)),
            "memtracker_metrics.json",
        ),
        type=str,
    )
    parser.add_argument(
        "--run-seconds",
        help="Number of seconds to run for (0 = forever)",
        default=0,
        type=int,
    )
    parser.add_argument(
        "--output-summary",
        help="Filename to output derived summary values as JSON",
        type=str,
    )
    parser.add_argument(
        "--output-html",
        help="Filename to output graph HTML",
        type=str,
    )

    args = parser.parse_args()

    analyzer = Analyzer(args.host_only)
    if args.filename:
        # Parsing a previous log rather than capturing one.
        summary_entries = []

        def analyze():
            with open(args.filename, encoding="utf-8") as f:
                for line in f:
                    entry = json.loads(line)
                    summary = analyzer.handle_report(entry)
                    if summary:
                        summary_entries.append(summary)

        analyze()
        # Convenience shortcut: rerun the analysis in host-only mode
        # if the analysis returned nothing because the input data was
        # missing guest entries (i.e. was collected with --host-only).
        if (
            not summary_entries
            and not args.host_only
            and "guest" not in analyzer.sources()
        ):
            print("No guest entries found; redoing analysis with host-only.")
            analyzer = Analyzer(host_only=True)
            analyze()
        if not summary_entries:
            print("ERROR: No summary entries were produced.")
            return 1
        # Write out JSON summary if requested.
        if args.output_summary:
            with open(args.output_summary, "wt", encoding="utf-8") as f:
                json.dump(summary_entries, f)
        # Write out interactive chart HTML if requested.
        if args.output_html:
            with open(args.output_html, "wt", encoding="utf-8") as f:
                generate_html_from_summary(summary_entries, f)
    else:
        # Collecting a new log.
        if args.output_summary or args.output_html:
            print(
                "ERROR: --output-summary and --output-html only work"
                " when analyzing a previously captured log."
            )
            return 1
        tracker = CombinedTracker(
            analyzer,
            run_seconds=args.run_seconds,
            host_only=args.host_only,
            host_log_path=args.host_log_path,
        )
        return tracker.main()

    return 0


if __name__ == "__main__":
    sys.exit(main())
