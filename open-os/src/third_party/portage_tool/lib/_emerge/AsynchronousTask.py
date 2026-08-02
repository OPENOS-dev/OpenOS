# Copyright 1999-2018 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2

import contextvars
import signal

from portage import telemetry
from portage import os
from portage.util.futures import asyncio
from portage.util.SlotObject import SlotObject

tracer = telemetry.get_tracer(__name__)

TRACE_LIST = frozenset([
	"MergeListItem",
	"EbuildBuild",
	"PackageMerge",
	"Binpkg",
	"EbuildMerge"
	"EbuildFetcher",
	"EbuildFetchOnly",
	"PackageUninstall",
	"EbuildBinpkg",
	"BinpkgExtractorAsync",
])


class AsynchronousTask(SlotObject):
	"""
	Subclasses override _wait() and _poll() so that calls
	to public methods can be wrapped for implementing
	hooks such as exit listener notification.

	Sublasses should call self._async_wait() to notify exit listeners after
	the task is complete and self.returncode has been set.
	"""

	__slots__ = ("background", "cancelled", "returncode", "scheduler") + \
		("_exit_listeners", "_exit_listener_stack", "_start_listeners") + \
		("_exit_listener_contexts", "_start_listener_contexts")

	_cancelled_returncode = - signal.SIGINT

	def start(self):
		"""
		Start an asynchronous task and then return as soon as possible.

		Opentelemetry Span is used to track the lifecycle of the Task. We make
		a copy of the context invoking the start; the span contained in the
		context becomes the parent span. A new span is created and attached to
		the created context. The _start function is run inside this context
		ensuring that this span is available to _start function overridden by
		the child class. The context is also passed along to the exit listener
		that detaches the span from this context at the end of the task.
		"""
		self._start_hook()

		context = contextvars.copy_context()

		def _stop_span(token, returncode):
			span = telemetry.get_current_span()
			span.set_attribute("return_code", str(returncode))
			span.set_status(telemetry.StatusCode.ERROR if returncode != os.EX_OK else telemetry.StatusCode.OK)
			span.end()
			telemetry.detach_span(token)

		def _start_internal():
			name = self.__class__.__name__
			if name in TRACE_LIST:
				span = tracer.start_span(name)

				pkg = None
				if hasattr(self, "pkg"):
					pkg = self.pkg
				elif hasattr(self, "merge") and hasattr(self.merge, "pkg"):
					pkg = self.merge.pkg

				if pkg:
					span.set_attributes({
						"pkg": str(pkg.cpv),
						"pkg_parents": pkg.parents or [],
						"pkg_installed": str(pkg.installed),
						"pkg_type_name": str(pkg.type_name),
					})
				token = telemetry.attach_span(span)
				self.addExitListener(lambda self: _stop_span(token, self.returncode), context=context)
			self._start()

		context.run(_start_internal)

	def async_wait(self):
		"""
		Wait for returncode asynchronously. Notification is available
		via the add_done_callback method of the returned Future instance.

		@returns: Future, result is self.returncode
		"""
		waiter = self.scheduler.create_future()
		exit_listener = lambda self: waiter.set_result(self.returncode)
		self.addExitListener(exit_listener)
		waiter.add_done_callback(lambda waiter:
			self.removeExitListener(exit_listener) if waiter.cancelled() else None)
		if self.returncode is not None:
			# If the returncode is not None, it means the exit event has already
			# happened, so use _async_wait() to guarantee that the exit_listener
			# is called. This does not do any harm because a given exit listener
			# is never called more than once.
			self._async_wait()
		return waiter

	def _start(self):
		self.returncode = os.EX_OK
		self._async_wait()

	def isAlive(self):
		return self.returncode is None

	def poll(self):
		if self.returncode is not None:
			return self.returncode
		self._poll()
		self._wait_hook()
		return self.returncode

	def _poll(self):
		return self.returncode

	def wait(self):
		"""
		Wait for the returncode attribute to become ready, and return
		it. If the returncode is not ready and the event loop is already
		running, then the async_wait() method should be used instead of
		wait(), because wait() will raise asyncio.InvalidStateError in
		this case.

		@rtype: int
		@returns: the value of self.returncode
		"""
		if self.returncode is None:
			if self.scheduler.is_running():
				raise asyncio.InvalidStateError('Result is not ready.')
			self.scheduler.run_until_complete(self.async_wait())
		self._wait_hook()
		return self.returncode

	def _async_wait(self):
		"""
		For cases where _start exits synchronously, this method is a
		convenient way to trigger an asynchronous call to self.wait()
		(in order to notify exit listeners), avoiding excessive event
		loop recursion (or stack overflow) that synchronous calling of
		exit listeners can cause. This method is thread-safe.
		"""
		self.scheduler.call_soon(self.wait)

	def cancel(self):
		"""
		Cancel the task, but do not wait for exit status. If asynchronous exit
		notification is desired, then use addExitListener to add a listener
		before calling this method.
		NOTE: Synchronous waiting for status is not supported, since it would
		be vulnerable to hitting the recursion limit when a large number of
		tasks need to be terminated simultaneously, like in bug #402335.
		"""
		if not self.cancelled:
			self.cancelled = True
			self._cancel()

	def _cancel(self):
		"""
		Subclasses should implement this, as a template method
		to be called by AsynchronousTask.cancel().
		"""
		pass

	def _was_cancelled(self):
		"""
		If cancelled, set returncode if necessary and return True.
		Otherwise, return False.
		"""
		if self.cancelled:
			if self.returncode is None:
				self.returncode = self._cancelled_returncode
			return True
		return False

	def addStartListener(self, f, context=None):
		"""
		The function will be called with one argument, a reference to self.
		"""
		if self._start_listeners is None:
			self._start_listeners = []
			self._start_listener_contexts = {}
		self._start_listeners.append(f)
		self._start_listener_contexts[f] = context or contextvars.copy_context()

	def removeStartListener(self, f):
		if self._start_listeners is None:
			return
		self._start_listeners.remove(f)
		del self._start_listener_contexts[f]

	def _start_hook(self):
		if self._start_listeners is not None:
			start_listeners = self._start_listeners
			contexts = self._start_listener_contexts
			self._start_listeners = None
			self._start_listener_contexts = None

			for f in start_listeners:
				contexts[f].run(f, self)

	def addExitListener(self, f, context=None):
		"""
		The function will be called with one argument, a reference to self.
		"""
		if self._exit_listeners is None:
			self._exit_listeners = []
			self._exit_listener_contexts = {}
		self._exit_listeners.append(f)
		self._exit_listener_contexts[f] = context or contextvars.copy_context()

	def removeExitListener(self, f):
		if self._exit_listeners is None:
			if self._exit_listener_stack is not None:
				self._exit_listener_stack.remove(f)
			return
		self._exit_listeners.remove(f)
		del self._exit_listener_contexts[f]

	def _wait_hook(self):
		"""
		Call this method after the task completes, just before returning
		the returncode from wait() or poll(). This hook is
		used to trigger exit listeners when the returncode first
		becomes available.
		"""
		if self.returncode is not None and \
			self._exit_listeners is not None:

			# This prevents recursion, in case one of the
			# exit handlers triggers this method again by
			# calling wait(). Use a stack that gives
			# removeExitListener() an opportunity to consume
			# listeners from the stack, before they can get
			# called below. This is necessary because a call
			# to one exit listener may result in a call to
			# removeExitListener() for another listener on
			# the stack. That listener needs to be removed
			# from the stack since it would be inconsistent
			# to call it after it has been been passed into
			# removeExitListener().
			self._exit_listener_stack = self._exit_listeners
			contexts = self._exit_listener_contexts
			self._exit_listener_contexts = None
			self._exit_listeners = None

			# Execute exit listeners in reverse order, so that
			# the last added listener is executed first. This
			# allows SequentialTaskQueue to decrement its running
			# task count as soon as one of its tasks exits, so that
			# the value is accurate when other listeners execute.
			while self._exit_listener_stack:
				fn = self._exit_listener_stack.pop()
				contexts[fn].run(fn, self)
