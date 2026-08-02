# Copyright 2023 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Telemetry module to provide tracing to portage.

The module provides the common functions to be used for telemetry generation
in portage by wrapping the opentelemetry operations. Operations inject no op
implementations of Tracer and Span if Opentelemetry libs are not present.

The api for the no op implementations is kept consistent with opentelemetry
equivalent classes.
"""

import contextlib
from datetime import datetime
import enum
import os
from pathlib import Path
from typing import Optional
import uuid

from portage.util import normalize_path

USE_TELEMETRY = False
_HAS_OTEL = None


class StatusCode(enum.Enum):
	"""Mirrors the opentelemetry.trace.StatusCode"""
	UNSET = 0
	OK = 1
	ERROR = 2


def init():
	"""Initialize the telemetry libs."""
	if not USE_TELEMETRY:
		return

	global _HAS_OTEL

	try:
		from opentelemetry import trace
		from opentelemetry.sdk import trace as trace_sdk
		from opentelemetry.sdk.trace import export
		from opentelemetry.sdk import resources
		# Transitive opentelemetry import.
		from portage import telemetry_detector
		from portage import telemetry_exporter
	except ImportError:
		_HAS_OTEL = False
		return
	else:
		_HAS_OTEL = True

	try:
		if "PORTAGE_LOGDIR" in os.environ and os.access(
			os.environ["PORTAGE_LOGDIR"],
			os.W_OK
		):
			logdir = Path(normalize_path(os.environ["PORTAGE_LOGDIR"]))
		else:
			EPREFIX = Path(
				normalize_path(os.environ.get('PORTAGE_OVERRIDE_EPREFIX', '/'))
			)
			logdir = EPREFIX / "var" / "log" / "portage"

		# Copy the basic telemetry log directory/file format chromite uses.
		logdir /= "telemetry"

		now = datetime.now()
		logdir /= now.strftime("%Y-%m-%d")

		# We only instrument emerge, so just hardcode it for now.
		cmd = "emerge"
		cmd_time = now.strftime("%H_%M_%S")
		logdir /= f'{cmd_time}-{cmd}'

		file_uuid = str(uuid.uuid4())
		traces_file = logdir / f'{file_uuid}.otel.traces.json'
		traces_file.parent.mkdir(parents=True, exist_ok=True)
	except Exception:
		# Skip telemetry if we can't generate a path to put it in.
		return

	try:
		exporter = telemetry_exporter.ChromiteFileExporter(traces_file)
	except OSError:
		# Just in case we can't open the trace file.
		return

	resource = resources.get_aggregated_resources(
		[
			resources.ProcessResourceDetector(),
			resources.OTELResourceDetector(),
			telemetry_detector.ProcessDetector(),
			telemetry_detector.SystemDetector(),
		]
	)
	trace.set_tracer_provider(trace_sdk.TracerProvider(resource=resource))
	trace.get_tracer_provider().add_span_processor(
		export.SimpleSpanProcessor(exporter)
	)


def get_tracer(name: str, version: Optional[str] = None):
	"""Return opentelemetry tracer if present."""
	global _HAS_OTEL
	if USE_TELEMETRY and _HAS_OTEL is not False:
		try:
			from opentelemetry import trace
		except ImportError:
			_HAS_OTEL = False
		else:
			return trace.get_tracer(name, version)
	return NoOpTracer()


def get_current_span():
	"""Return current opentelemetry span if present."""
	global _HAS_OTEL
	if USE_TELEMETRY and _HAS_OTEL is not False:
		try:
			from opentelemetry import trace
		except ImportError:
			_HAS_OTEL = False
		else:
			return trace.get_current_span()
	return NoOpSpan()


def attach_span(span):
	"""Attach the span to current context."""
	global _HAS_OTEL
	if USE_TELEMETRY and _HAS_OTEL is not False:
		try:
			from opentelemetry import context
			from opentelemetry import trace
		except ImportError:
			_HAS_OTEL = False
		else:
			return context.attach(context.set_value(trace._SPAN_KEY, span))
	return span


def detach_span(token):
	"""Detach the current span for the token."""
	global _HAS_OTEL
	if USE_TELEMETRY and _HAS_OTEL is not False:
		try:
			from opentelemetry import context
		except ImportError:
			_HAS_OTEL = False
		else:
			context.detach(token)

class NoOpSpan:
	"""NoOpSpan to model opentelemetry.trace.Span."""

	def record_exception(self, *args) -> None:
		pass

	def is_recording(self) -> bool:
		return True

	def add_event(self, *args) -> None:
		pass

	def set_attribute(self, *args) -> None:
		pass

	def set_attributes(self, *args) -> None:
		pass

	def end(self, *args) -> None:
		pass

	def set_status(self, *args) -> None:
		pass

	def __enter__(self):
		return self

	def __exit__(self, *args):
		pass


class NoOpTracer:
	"""NoOpTracer to model opentelemetry.trace.Tracer."""

	def start_span(self, _name: str) -> NoOpSpan:
		"""Start span."""
		return NoOpSpan()

	@contextlib.contextmanager
	def start_as_current_span(self, name: str):
		"""Start as current span."""
		yield self.start_span(name)
