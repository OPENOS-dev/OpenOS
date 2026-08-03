# Copyright (c) 2013 The Chromium OS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# This module contains unit tests for xorg_parser.
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

from os import path
from xorg_parser import XorgInputClassParser
import unittest

class XorgInputClassParserTests(unittest.TestCase):
  def test_sections(self):
    conf = ("Section \"InputClass\"\n" +
            "EndSection\n" +
            "Section \"InputClass\"\n" +
            "Identifier \"TestClass\"\n" +
            "EndSection\n" +
            "Section \"SectionB\"\n" +
            "EndSection\n")

    parser = XorgInputClassParser()
    result = parser.Parse(string=conf)

    self.assertEqual(len(result), 1)
    self.assertTrue("TestClass" in result)

  def test_comments(self):
    conf = (" # test\n" +
            "Section \"InputClass\"\n" +
            "# EndSection\n" +
            "Identifier \"TestClass\"\n" +
            "EndSection\n")

    parser = XorgInputClassParser()
    result = parser.Parse(string=conf)

    self.assertEqual(len(result), 1)
    self.assertTrue("TestClass" in result)

  def test_options(self):
    conf = ("Section \"InputClass\"\n" +
            "Identifier \"ClassA\"\n" +
            "Option \"A\" \"valueA\"\n" +
            "Option \"B\" \"valueB\"\n" +
            "EndSection\n" +
            "Section \"InputClass\"\n" +
            "Identifier \"ClassB\"\n" +
            "Option \"C\" \"valueC\"\n" +
            "Option \"D\" \"valueD\"\n" +
            "EndSection\n")

    parser = XorgInputClassParser()
    result = parser.Parse(string=conf)

    expected = {}
    expected["ClassA"] = {"A": "valueA", "B": "valueB"}
    expected["ClassB"] = {"C": "valueC", "D": "valueD"}

    self.assertEqual(result, expected)

  def test_real_config_file(self):
    filename = path.join(path.dirname(__file__), "xorg_parser_tests.conf")

    parser = XorgInputClassParser()
    result = parser.Parse(file=filename)

    expected = {}
    expected["SectionA"] = {"A": "valueA", "B": "valueB"}
    expected["SectionB"] = {"C": "valueC", "D": "valueD"}

    self.assertEqual(len(result), 3)
    self.assertTrue("touchpad lumpy" in result)
    self.assertTrue("touchpad lumpy atmel" in result)
    self.assertTrue("touchpad lumpy cyapa" in result)

if __name__ == '__main__':
  unittest.main()
