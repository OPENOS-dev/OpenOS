# Copyright (c) 2013 The Chromium OS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# This module imports all test cases in this project and executes them.
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

from fuzzy_check_tests import FuzzyCheckDragTests
from fuzzy_check_tests import FuzzyCheckTests
from fuzzy_check_tests import FuzzyComparatorTests
from gesture_log_tests import GestureLogRoughnessTests
from gesture_log_tests import GestureLogTests
from test_case_tests import TestCaseTests
from test_factory_tests import TestFactoryTests
from test_runner_tests import TestRunnerTests
from xorg_parser_tests import XorgParserTests
import unittest

if __name__ == '__main__':
  unittest.main()
