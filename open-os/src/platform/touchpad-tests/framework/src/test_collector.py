# Copyright (c) 2014 The Chromium OS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

from collections import namedtuple
from itertools import chain
import os
import sys

from mtlib.cros_remote import CrOSRemote
from mtlib.log import DeviceLog
from mtlib.util import AskUser
from mtreplay import MTReplay
from test_case import TestCase
from test_factory import TestFactory
from test_runner import TestRunner


class LogCollector(object):
  def __init__(self, ip):
    self.ip = ip
    self.remote = CrOSRemote(self.ip)

    name = self.remote.GetDeviceInfo().board_variant
    self.spec_file = "device_specs/{}".format(name)

  def ClearLog(self):
    self.remote.Execute(
        "/opt/google/touchpad/tpcontrol set \"Logging Reset\" 1")

  def GetLog(self, platform):
    replay = MTReplay()

    log = DeviceLog(self.ip, new=True)
    return replay.TrimEvdev(log, force_platform=platform)

class TestCollector(object):
  def __init__(self, ip, tests_dir):
    self.tests_dir = tests_dir
    self.collector = LogCollector(ip)

  def GenerateTestCase(self, test_case):
    self.collector.ClearLog()
    print("Perform case", test_case.name)
    description = test_case.user_description
    if description:
      print(description)
    print("Hit enter when done.")
    AskUser.Continue()
    log = self.collector.GetLog(test_case.platform_name);
    if not log:
      print("Nothing happened... Did you do the gesture?")
      return False

    factory = TestFactory(self.tests_dir)
    case = factory.CreateTest(test_case.name, log, False)
    runner = TestRunner(os.environ["TESTS_DIR"])
    if case.takes_original_values:
      print("need to setup original values")
      out = runner.RunTest(case, generate_original_values=True)
      if out["score"] > 0.0:
        # Rewrite case with original values
        case = factory.CreateTest(test_case.name, log, False,
                                  original_values=out["original_values"])
      else:
        print("Failed:", out["logs"]["validation"])
        return False
    out = runner.RunTest(case)
    if not out["score"] > 0.0:
      print(out["logs"]["validation"])
      return False
    return True

  def GenerateAll(self, glob, overwrite=False):
    cases = TestCollector.DiscoverCollectableCases(self.tests_dir, glob)
    if not overwrite:
      cases = filter(lambda c: not c.IsComplete(), cases)

    print("You will collect the following cases:")
    for case in cases:
      print("  - %s" % (case.name))
    AskUser.Continue()

    for case in cases:
      max_attempts = 3
      for attempt in range(max_attempts):
        print("Attempt", (attempt + 1), "of", max_attempts)
        if self.GenerateTestCase(case):
          print("Success!")
          break
        print("Failure.")

  @staticmethod
  def DiscoverCollectableCases(tests_dir, glob=None):
    cases = TestCase.DiscoverTestCases(tests_dir, glob)
    return filter(lambda c: c.user_description is not None, cases)
