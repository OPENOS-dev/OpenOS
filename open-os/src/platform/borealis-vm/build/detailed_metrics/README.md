# Borealis Detailed/Debug/High Volume Metric Collector

[go/borealis-high-volume-metrics](http://go/borealis-high-volume-metrics)

_This is not the same thing as /usr/bin/metric_collector.py, which collects
anonymous metrics at a much slower rate, and automatically reports them as
Chrome OS histograms._

This program collects high frequency (~1/s) metrics on various interesting
things, plus log snippets and game information.  The data is written into a file
in /tmp, which will be picked up by `bdt report -u`.

Usage:

- `detailed_metrics` - output various metrics to /tmp/metrics.json.

- `detailed_metrics --quick-stats` - output a selection of useful numbers in JSON format (one struct per line) to stdout and /tmp/metrics.json.
