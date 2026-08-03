# Copyright (c) 2012 The Chromium OS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# This module contains unit tests for TestRunner
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

from os import path
from test_runner import TestRunner
import unittest

tests_dir = path.join(path.dirname(__file__), "testsdir")
replay_tool = path.join(path.dirname(__file__), "../../../replay/build/replay")

class TestRunnerTests(unittest.TestCase):
  """
  Unit tests for TestRunner. These tests will create a new test runner and check
  if all tests from testsdir can be discovered and executed.
  """

  def test_platform_detection(self):
    platforms = TestRunner(tests_dir, replay_tool)._DiscoverPlatforms()
    self.assertEqual(set(platforms), set(["platform_a", "platform_b"]))

  def test_case_discovery(self):
    cases = TestRunner(tests_dir, replay_tool).DiscoverTestCases()
    case_names = map(lambda t: t.name, cases)
    self.assertTrue("platform_a/complete_test" in case_names)
    self.assertTrue("platform_a/fallback_test" in case_names)
    self.assertTrue("platform_a/category_a/complete_test" in case_names)
    self.assertTrue("platform_a/category_a/fallback_test" in case_names)

  def test_case_wildcard_discovery(self):
    runner = TestRunner(tests_dir, replay_tool)
    cases = runner.DiscoverTestCases("platform_a/category_a/*")
    case_names = map(lambda t: t.name, cases)
    self.assertFalse("platform_a/complete_test" in case_names)
    self.assertFalse("platform_a/fallback_test" in case_names)
    self.assertTrue("platform_a/category_a/complete_test" in case_names)
    self.assertTrue("platform_a/category_a/fallback_test" in case_names)

  def test_single_test_discovery(self):
    runner = TestRunner(tests_dir, replay_tool)
    cases = runner.DiscoverTestCases("platform_a/category_a/complete_test")
    self.assertEqual(len(cases), 1)
    self.assertEqual(cases[0].name, "platform_a/category_a/complete_test")

  def test_run_results_listed(self):
    results = TestRunner(tests_dir, replay_tool).RunAll()

    # check if test cases are included in results
    self.assertTrue("platform_a/complete_test" in results.keys())
    self.assertTrue("platform_a/fallback_test" in results.keys())
    self.assertTrue("platform_a/category_a/complete_test" in results.keys())
    self.assertTrue("platform_a/category_a/fallback_test" in results.keys())

  def test_run_correct_results(self):
    results = TestRunner(tests_dir, replay_tool).RunAll()

    # check if test cases are included in results
    self.assertEqual(results["platform_b/failing_test"]["result"], "failure")
    self.assertEqual(results["platform_b/broken_module"]["result"], "error")
    self.assertEqual(results["platform_b/no_validate"]["result"], "error")

if __name__ == '__main__':
  unittest.main()
