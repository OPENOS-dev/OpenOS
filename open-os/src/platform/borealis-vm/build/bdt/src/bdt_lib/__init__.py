# Copyright 2022 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Internals of the Borealis Dev Tool, /usr/bin/bdt."""

import unittest

from . import commands
from . import quirks
from . import test_commands
from . import test_quirks
from . import test_window_trace
from . import upload
from . import window_trace


# pylint: disable=unused-argument
def load_tests(loader, tests, pattern):
    """Assist unittest discovery.

    See https://docs.python.org/3/library/unittest.html#load-tests-protocol
    """
    suite = unittest.TestSuite()
    suite.addTests(loader.loadTestsFromModule(test_commands))
    suite.addTests(loader.loadTestsFromModule(test_quirks))
    suite.addTests(loader.loadTestsFromModule(test_window_trace))
    return suite
