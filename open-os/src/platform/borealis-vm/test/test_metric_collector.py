# Copyright 2022 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Tests for /usr/bin/metric_collector.py."""

import subprocess
import sys
import unittest
from unittest import mock


# Hackily import /usr/bin/metric_collector as a module.
sys.path.insert(0, "/usr/bin")
import metric_collector  # pylint: disable=import-error,wrong-import-position


# pylint: disable=no-self-use  # Test cases don't always call self.*.


class TestMetricCollector(unittest.TestCase):
    """Test the metric collector script."""

    xrun_data = "state: RUNNING\n-----\nxrun_counter: 1\n"

    def test_collect_dirty_pages(self):
        """Test that collect_dirty_pages adds to |metrics|."""
        metrics = []
        metric_collector.collect_dirty_pages(metrics)
        self.assertTrue(metrics)

    def test_read_vmstat(self):
        """Test that read_vmstat produces data."""
        vmstat = metric_collector.read_vmstat()
        for src in ("swp", "gpg"):
            for rw in ("in", "out"):
                self.assertIsNotNone(vmstat[f"p{src}{rw}"])

    def test_collect_diff(self):
        """Test that collect_diff adds to |metrics|."""
        metrics = []
        metric_collector.collect_diff(
            metrics,
            # vmstat
            {"pswpin": 123, "pswpout": 456, "pgpgin": 789, "pgpgout": 1000},
            # laststat
            {"pswpin": 23, "pswpout": 256, "pgpgin": 489, "pgpgout": 600},
        )
        self.assertListEqual(
            metrics,
            [
                "borealis-disk-kb-read=1200",
                "borealis-disk-kb-written=1600",
                "borealis-swap-kb-read=400",
                "borealis-swap-kb-written=800",
            ],
        )

    @mock.patch(
        "builtins.open", new_callable=mock.mock_open, read_data="closed\n"
    )
    def test_collect_direct_alsa_xrun_closed(self, mock_check_call):
        """Test collect xrun when the device is closed"""
        metrics = []
        metric_collector.collect_direct_alsa_xrun(
            metrics, {"xrun_counter_0": 0}, 1
        )
        mock_check_call.assert_called_once_with(
            "/proc/asound/card0/pcm0p/sub0/status", encoding="utf-8"
        )
        self.assertListEqual(metrics, [])

    @mock.patch(
        "builtins.open", new_callable=mock.mock_open, read_data=xrun_data
    )
    def test_collect_direct_alsa_xrun_running(self, mock_check_call):
        """Test collect xrun when the device is running"""
        metrics = []
        metric_collector.collect_direct_alsa_xrun(
            metrics, {"xrun_counter_0": 0}, 1
        )
        mock_check_call.assert_called_once_with(
            "/proc/asound/card0/pcm0p/sub0/status", encoding="utf-8"
        )
        self.assertListEqual(metrics, ["borealis-audio-xrun-alsa-output=1"])

    @mock.patch("subprocess.check_call")
    def test_send_metrics(self, mock_check_call):
        """Test that collected metrics are sent to garcon correctly."""
        metrics = ["test=0", "metric3=2", "test-metric=5000"]
        metric_collector.send_metrics(metrics, dry_run=False)
        mock_check_call.assert_called_once_with(
            [
                "/opt/google/cros-containers/bin/garcon",
                "--client",
                "--metrics",
                "test=0,metric3=2,test-metric=5000",
            ]
        )

    def test_executable(self):
        """Test that /usr/bin/metric_collector.py is runnable."""
        subprocess.check_call(
            [
                "/usr/bin/metric_collector.py",
                "--verbose",
                "--test-disable-syslog",
                "--test-single-run",
            ]
        )
