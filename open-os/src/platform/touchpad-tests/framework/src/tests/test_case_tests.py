# Copyright (c) 2012 The Chromium OS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# This module contains unit tests for TestCase.
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

from os import path
from test_case import TestCase
import unittest

tests_path = path.join(path.dirname(__file__), "testsdir")

class TestCaseTests(unittest.TestCase):
  """
  Unit tests for TestCase. In the testsdir folder is a set of test definitions,
  some correct, some with errors.
  These unit tests will check if all of these tests are recognized correctly
  and if the tests are linking to the right files.
  """

  def test_naming(self):
    case = TestCase(tests_path, "platform_a/complete_test.log")
    self.assertEqual(case.name, "platform_a/complete_test")

  def test_case_files(self):
    testcase = TestCase(tests_path, "platform_a/complete_test.log")
    self.assertEqual(testcase.platform_dat_file,
                     tests_path + "/platform_a/platform.dat")
    self.assertEqual(testcase.log_file,
                     tests_path + "/platform_a/complete_test.log")
    self.assertTrue(testcase.module is not None)
    self.assertEqual(testcase.module.__file__,
                     tests_path + "/platform_a/complete_test.pyc")
    self.assertTrue(testcase.module.Validate is not None)

  def test_case_fallback_files(self):
    testcase = TestCase(tests_path, "platform_a/fallback_test.log")
    self.assertEqual(testcase.platform_dat_file,
                     tests_path + "/platform_a/platform.dat")
    self.assertEqual(testcase.log_file,
                     tests_path + "/platform_a/fallback_test.log")
    self.assertTrue(testcase.module is not None)
    self.assertEqual(testcase.module.__file__,
                     tests_path + "/common/fallback_test.pyc")
    self.assertTrue(testcase.module.Validate is not None)

  def test_case_validation(self):
    def assertCheckOK(name):
      self.assertTrue(TestCase(tests_path, name).Check() is True)

    def assertCheckError(name):
      self.assertTrue(TestCase(tests_path, name).Check() is not True)

    assertCheckOK("platform_a/complete_test")
    assertCheckOK("platform_a/fallback_test")
    assertCheckOK("platform_a/category_a/complete_test")
    assertCheckOK("platform_a/category_a/fallback_test")

    assertCheckError("non/existent/test_case")
    assertCheckError("platform_b/no_module")
    assertCheckError("platform_b/broken_module")
    assertCheckError("platform_b/no_check")


if __name__ == '__main__':
  unittest.main()
