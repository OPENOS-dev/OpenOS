# Copyright (c) 2012 The Chromium OS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# This module provides the TestVerifier class which is used to verify that
# replay is working correctly by comparing the simulated replay with a replay
# through the actual kernel device on the target machine.
#
# This module is DEPRECATED!

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

from replay import Replay
from test_case import TestCase
from test_factory import TestFactory
from test_runner import TestRunner
import decimal
import json
import math
import os

def safecall(cmd):
  """
  Helper method that raises an exception when a shell command fails
  """
  result = os.system(cmd)
  if result != 0:
    raise Exception("Command failed: " + cmd)

class TestVerifier(object):
  """
  This class provides the functionality to verify a test case. Verification
  in this context means that the log is replayed on the real kernel device
  as well as in the test framework. The results of both replays are then
  compared to locate any mismatches.
  """

  def __init__(self, tests_dir, replay_tool, device_name):
    """
    The constructor will poll the current values of the user defined properties
    in order to restore them after each verification run.
    """
    self.replay_tool = replay_tool
    self.device_name = device_name
    self.runner = TestRunner(tests_dir, replay_tool)

    # We need to store the user defined properties to reset them later
    self.original_props = {}
    safecall("/opt/google/touchpad/tpcontrol log")
    decimal.setcontext(decimal.Context(prec=8))
    activity = json.load(open("/var/log/touchpad_activity_log.txt", "r"))
    for userprop in TestFactory.user_property_names:
      if userprop in activity["properties"]:
        self.original_props[userprop] = activity["properties"][userprop]

  def Verify(self, case):
    """
    Verifies a test case. This method will first setup all user defined
    properties to match the ones recorded with the test case, and then
    use evemu-play to replay the log.
    Afterwards the log is replayed using the test framework and both results
    are compared. Finally all user defined properties are restored to their
    original values.
    """
    # set user defined properties
    properties = case.gesture_props
    for userprop in TestFactory.user_property_names:
      if userprop in properties:
        value = properties[userprop]
        safecall("xinput set-prop `/opt/google/touchpad/tpcontrol listdev` \""
                 + userprop + "\" \"" + str(value) + "\"")

    # pass test event file through kernel device and collect log
    safecall("/opt/google/touchpad/tpcontrol log-reset")
    safecall("evemu-play " + self.device_name + " < " + case.log_file)
    safecall("/opt/google/touchpad/tpcontrol log")
    real = json.load(open("/var/log/touchpad_activity_log.txt", "r"))

    # simulated replay using the test framework
    result = self.runner.RunTest(case)
    if result["error"]:
      return "Failed to execute test: " + result["error"]
    fake = json.loads(result["logs"]["activity"], parse_float=decimal.Decimal)

    # verify and collect results for report
    report = self._VerifyProperties(fake, real)
    report = report + self._VerifyHWStates(fake, real)
    report = report + self._VerifyGestures(fake, real)

    # restore original properties
    properties = case.gesture_props
    for key, value in self.original_props.items():
      safecall("xinput set-prop `/opt/google/touchpad/tpcontrol listdev` \""
               + key + "\" \"" + str(value) + "\"")

    return report

  def _VerifyProperties(self, fake, real):
    report = []
    fake_props = fake["properties"]

    for property, value in real["properties"].items():
      if property not in fake_props:
        report.append(property + ": not present in simulation")
        continue

      valid = True
      # validate the property value (floats only need to be almost equal)
      if isinstance(value, float) and isinstance(fake_props[property], float):
        delta = math.fabs(value - fake_props[property])
        if delta > 1e-10:
          valid = False
      elif fake_props[property] != value:
        valid = False

      if not valid:
        report.append(property + ": " + str(value) + " != "
                      + str(fake_props[property]))

    # build final report presentation
    if report:
      return "Property mismatches: \n    " + "\n    ".join(report) + "\n"
    return ""

  def _VerifyHWStates(self, fake, real):
    report = []

    # filter hardware states from logs
    real_hwstates = filter(lambda e: e["type"] == "hardwareState",
                           real["entries"])
    fake_hwstates = filter(lambda e: e["type"] == "hardwareState",
                           fake["entries"])

    # validate all states
    for i in range(len(real_hwstates)):
        if i >= len(fake_hwstates):
          break;

        real_hwstate = real_hwstates[i]
        fake_hwstate = fake_hwstates[i]

        # helper method to add a report entry for the current hardware state
        def ReportMismatch(name, fake_value, real_value):
          tag = str(real_hwstate["timestamp"]) + ": " + name + " "
          report.append(tag + str(real_value) + " != " + str(fake_value))

        # check touch count. Don't check any other values if they don't match
        if real_hwstate["touchCount"] != fake_hwstate["touchCount"]:
          ReportMismatch("touchCount", fake_hwstate["touchCount"],
                         real_hwstate["touchCount"])
          continue

        # In some cases touchCount != len(fingers). Check this too
        if len(real_hwstate["fingers"]) != len(fake_hwstate["fingers"]):
          ReportMismatch("len(fingers)", len(fake_hwstate["fingers"]),
                         len(real_hwstate["fingers"]))
          continue

        # verify all fingers
        for j in range(len(real_hwstate["fingers"])):
          real_finger = real_hwstate["fingers"][j]
          fake_finger = fake_hwstate["fingers"][j]

          # helper method to verify a finger field
          def VerifyField(field):
            if real_finger[field] != fake_finger[field]:
              ReportMismatch(field, real_finger[field], fake_finger[field])

          for key in fake_finger.keys():
            VerifyField(key)

    # if the real replay caused more hardware states than the simulated one
    # it is possible that the user touched the touchpad while the test was
    # running, causing additional events and messing up the test.
    if len(real_hwstates) != len(fake_hwstates):
      report.append("Count of hwstates mismatch!"
                    + "Did you touch the touchpad while running the test?")

    # build final report presentation
    if report:
      return "Hardware state mismatches: \n    " + "\n    ".join(report) + "\n"
    return ""

  def _VerifyGestures(self, fake, real):
    report = []

    # filter gestures from log
    real_gestures = filter(lambda e: e["type"] == "gesture", real["entries"])
    fake_gestures = filter(lambda e: e["type"] == "gesture", fake["entries"])

    # compare all gestures
    for i in range(len(real_gestures)):
      if i >= len(fake_gestures):
        break;

      real_gesture = real_gestures[i]
      fake_gesture = fake_gestures[i]

      # helper add an entry to the report for the current gesture
      def ReportMismatch(name, fake_value, real_value):
        tag = str(real_gesture["startTime"]) + ": " + name + " "
        report.append(tag + str(real_value) + " != " + str(fake_value))

      # check only the gesture type.
      if real_gesture["gestureType"] != fake_gesture["gestureType"]:
        ReportMismatch("gestureType", real_gesture["gestureType"],
                       fake_gesture["gestureType"])
        break

      # todo(denniskempin) check other gesture values too

    # check number of gestures
    if len(real_gestures) != len(fake_gestures):
      report.append("Count of gestures mismatch!")

    # build final report presentation
    if report:
      return "Gestures mismatches: \n    " + "\n    ".join(report) + "\n"
    return ""
