# Copyright (c) 2014 The Chromium OS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

from collections import namedtuple
from itertools import chain
import sys

from mtlib.cros_remote import CrOSRemote
from mtlib.log import DeviceLog
from mtreplay import MTReplay
from test_case import TestCase
from test_factory import TestFactory


def UserConfirm(msg=None):
  if msg:
    print(msg)
  print("Are you sure you wish to continue? [y/N]")
  response = sys.stdin.readline()
  if response.strip() not in ('y', 'Y', 'yes', 'Yes'):
    sys.exit(1)


class TestRobot(object):
  def __init__(self, ip, calib, slow=None, manual_fingertips=False):
    self.ip = ip
    self.slow = slow
    self.manual_fingertips = manual_fingertips
    self.remote = CrOSRemote(self.ip)
    self.current_finger_size = None
    self.current_tip_mask = None
    self.dir = self.FindRobotDir()

    name = self.remote.GetDeviceInfo().board_variant
    self.spec_file = "device_specs/{}".format(name)

    if not calib:
      if not self.remote.Execute(["test", "-e", self.spec_file + ".p"],
                                 cwd=self.dir) is not False:
        calib = True

    if calib:
      UserConfirm("Starting calibration for new device")
      cmd = "python calibrate_for_new_device.py " + self.spec_file
      if manual_fingertips:
        cmd = cmd + " --manual-fingertips"
      self.Execute(cmd)

  def FindRobotDir(self):
    search_dirs = [
      "/usr/lib64/python2.7/site-packages/touchbotII",
      "/usr/lib/python2.7/site-packages/touchbotII",
      "/usr/local/lib64/python2.7/site-packages/touchbotII",
      "/usr/local/lib/python2.7/site-packages/touchbotII"]
    for dir in search_dirs:
      if self.remote.Execute("test -e %s/calibrate_for_new_device.py"
                             % dir) is not False:
        return dir
    print("Cannot find robot directory")
    sys.exit(1)

  def CheckFingersOn(self):
    if not self.current_finger_size and not self.manual_fingertips:
      print("Select finger size before gesture")
      sys.exit(1)

  def CheckFingersOff(self):
    if self.current_finger_size and not self.manual_fingertips:
      print("Select finger size before gesture")
      sys.exit(1)

  def CleanUp(self):
    if self.manual_fingertips:
      return

    if self.current_finger_size is not None:
      print("### Removing finger tips (size %d)" % self.current_finger_size)
      self.Execute("python manipulate_fingertips.py %s %d False --fast-check"
                   % (self.current_tip_mask, self.current_finger_size))
    self.current_finger_size = None
    self.current_tip_mask = None

  def SetupFingerSize(self, size, tips=(1, 1, 1, 1)):
    if self.manual_fingertips:
      return
    mask = " ".join(map(lambda i: str(i), tips))
    fast_check = ""
    if self.current_finger_size != size or self.current_tip_mask != mask:
      if (self.current_finger_size is not None
          and self.current_tip_mask is not None):
        print("### Removing finger tips (size %d)" % self.current_finger_size)
        self.Execute("python manipulate_fingertips.py %s %d False --fast-check"
                     % (self.current_tip_mask, self.current_finger_size))
        fast_check = "--fast-check"

      print("### Picking finger tips (size %d)" % size)
      self.Execute("python manipulate_fingertips.py %s %d True %s"
                   % (mask, size, fast_check))
      self.current_finger_size = size
      self.current_tip_mask = mask

  def Line(self, start, end, tips, speed, type, delay=0.0):
    self.CheckFingersOn()
    if self.slow:
      speed = 5
    self.ExecuteScript("line.py", start, end, tips, (speed, type, delay))

  def Tap(self, pos, tips, count, delay=0.0):
    self.CheckFingersOn()
    self.ExecuteScript("tap.py", pos, tips, (count, "tap", delay))

  def Click(self, pos, tips, size=1):
    self.CheckFingersOff()
    self.SetupFingerSize(size, tips)
    self.ExecuteScript("click.py", pos, tips)
    self.CleanUp()

  def Execute(self, cmd):
    return self.remote.SafeExecute(cmd, cwd=self.dir, interactive=True)

  def ExecuteScript(self, file, *param_tuples):
    base = ("python", file, self.spec_file + ".p")
    args = map(lambda s: str(s), chain(base, *param_tuples))
    cmd = " ".join(args)
    self.Execute(cmd)

  def ClearLog(self):
    self.remote.Execute(
        "/opt/google/touchpad/tpcontrol set \"Logging Reset\" 1")

  def GetLog(self):
    replay = MTReplay()

    log = DeviceLog(self.ip, new=True)
    return replay.TrimEvdev(log)

class RobotTestGenerator(object):
  def __init__(self, ip, calib, slow, manual_fingertips, tests_dir):
    self.tests_dir = tests_dir
    self.robot = TestRobot(ip, calib, slow, manual_fingertips)

  def GenerateCase(self, test_case):
    print("Instructing robot to generate test", test_case.name)

    self.robot.ClearLog()
    test_case.instruct_robot(self.robot)
    log = self.robot.GetLog()
    if not log:
      print("Nothing happened... is the robot set up?")
      return

    factory = TestFactory(self.tests_dir)
    case = factory.CreateTest(test_case.name, log, False)

  def GenerateAll(self, glob, overwrite=False):
    cases = RobotTestGenerator.DiscoverRobotCases(self.tests_dir, glob)
    if not overwrite:
      cases = filter(lambda c: not c.IsComplete(), cases)

    expanded = []
    for case in cases:
      if case.is_virtual:
        continue
      # per default tests are performed with the finger size 1
      if hasattr(case.instruct_robot, "finger_size"):
        size = case.instruct_robot.finger_size
      else:
        size = 1
      expanded.append((TestCase(self.tests_dir, case.name), size))
    expanded.sort(key=lambda entry: entry[1])

    print("Instructing robot to generate the following tests:")
    for case, fingersize in expanded:
      if fingersize is None:
        print("  - %s (custom)" % (case.name))
      else:
        print("  - %s (%d)" % (case.name, fingersize))
    UserConfirm()

    # run tests with configured finger sizes
    for case, fingersize in expanded:
      if fingersize is None:
        continue
      self.robot.SetupFingerSize(fingersize)
      self.GenerateCase(case)

    self.robot.CleanUp()

    # run tests that have customized finger size setup
    for case, fingersize in expanded:
      if fingersize is None:
        self.GenerateCase(case)


  @staticmethod
  def DiscoverRobotCases(tests_dir, glob=None):
    cases = TestCase.DiscoverTestCases(tests_dir, glob)
    return filter(lambda c: c.instruct_robot is not None, cases)
