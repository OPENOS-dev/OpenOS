# Copyright 2024 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Chromite exporter.

See chromite.lib.telemetry.exporter.
"""

import os
from pathlib import Path

from opentelemetry.sdk.trace import export


class ChromiteFileExporter(export.SpanExporter):
	"""Chromite's span exporter."""

	def __init__(
		self,
		out_file: Path,
		formatter = lambda span: (span.to_json(indent=None) + os.linesep),
	):
		self.final_location = out_file
		self.in_progress = out_file.parent / f".{out_file.name}.in-progress"
		self._exporter = export.ConsoleSpanExporter(
			out=self.in_progress.open("w"), formatter=formatter
		)

	def export(self, spans) -> export.SpanExportResult:
		return self._exporter.export(spans)

	def shutdown(self) -> None:
		self._exporter.shutdown()
		if not self.in_progress.exists():
			# Just in case.
			return

		self.in_progress.rename(self.final_location)

	def force_flush(self, timeout_millis: int = 30000) -> bool:
		return self._exporter.force_flush(timeout_millis)
