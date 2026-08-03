# Copyright 2012 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# This module provides the TestFactory class to create new tests based upon
# an activity and cmt log.

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

from mtlib.gesture_log import GestureLog
from mtlib import Log
from mtreplay import MTReplay
from test_case import TestCase
import datetime
import decimal
import json
import math
import os
import shutil

new_test_template = """\
# Copyright %d The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from fuzzy_check import FuzzyCheck
from validators import *

# originally generated gestures:
%s

def Validate(raw, events, gestures):
  fuzzy = FuzzyCheck()
  fuzzy.expected = [
    # MotionValidator("== 100 ~ 50"),
    # FlingStopValidator(),
    # ButtonDownValidator(1),
    # ButtonUpValidator(1),
    # ScrollValidator(">= 100"),
    # FlingValidator(">= 100"),
    # AnythingButValidator(ButtonDownValidator(1)),
  ]
  fuzzy.unexpected = [
    # MotionValidator("<10"),
    # FlingStopValidator("<10"),
  ]
  return fuzzy.Check(gestures)
"""

class TestFactory(object):
  """
  The TestFactory can create new test cases. Testa are named after their
  activity log filename. It will automatically trim the cmt log to match
  the activity logs time range.
  """

  user_property_names = [
    "Tap Drag Enable",
    "Tap Enable"
  ]

  def __init__(self, tests_dir):
    self.tests_dir = tests_dir

  def _ValidateGestures(self, replay_log, original_log):
    if len(replay_log.gestures) != len(original_log.gestures):
      return False

    for i, replay_gesture in enumerate(replay_log.gestures):
      original_gesture = original_log.gestures[i]
      if original_gesture.type != replay_gesture.type:
        return False

    return True

  def CreateTest(self, name, log, gdb=False, original_values=None):
    """
    Create a new test. This method will create all necessary files to replay
    the events from the activity/cmt log file and also provides a template for
    the validator.
    """
    case = TestCase(self.tests_dir, name)

    test_folder = os.path.dirname(case.log_file)
    if not os.path.exists(test_folder):
      os.mkdir(test_folder)

    # make sure the precision is enough
    decimal.setcontext(decimal.Context(prec=8))
    activity = json.loads(log.activity, parse_float=decimal.Decimal)

    # extract user defined properties
    properties = activity["properties"]
    user_properties = {}
    user_property_names = list(TestFactory.user_property_names)

    if "Sensitivity" in properties:
      user_properties["Pointer Sensitivity"] = properties["Sensitivity"]
      user_properties["Scroll Sensitivity"] = properties["Sensitivity"]
    else:
      user_property_names.extend([
          "Pointer Sensitivity", "Scroll Sensitivity"])

    for name in user_property_names:
      if name not in case.ignore_properties:
        if name in properties:
          user_properties[name] = properties[name]
        else:
          print("Warning: No User-Property", name, "set")

    props = {"gestures": user_properties}
    if original_values is not None:
      props['original_values'] = original_values
    json.dump(props, open(case.case_property_file, "w"))
    case.ReloadProperties()

    # copy event log file
    open(case.log_file, "w").write(log.evdev)

    # test replay
    replay = MTReplay()
    replay_results = replay.Replay(log, case.gesture_props,
                                   case.platform)
    replay_log = replay_results.gestures

    # only create new module if this test case is not using the module
    # from commons
    if not case.module:
      report = "# " + "\n#   ".join([str(g) for g in replay_log.gestures])
      year = datetime.date.today().year
      open(case.case_module_file, "w").write(new_test_template % (year, report))

    self._CheckCreatedTest(case, log.activity, replay_log)

    return case

  def _CheckCreatedTest(self, case, activity_log, replay_log):
    original_log = GestureLog(activity_log)

    print("Checking test for warnings:")

    def PrintMismatch(type, key, original, replay):
      format = "  %s '%s' mismatch: Expected %s but is %s"
      print(format % (type, key, str(original), str(replay)))

    # check if device properties are the same
    property_errors = False

    properties_to_check = {}
    if os.path.exists(case.device_property_file):
      device_properties = json.load(open(case.device_property_file))
      if "gestures" in device_properties:
        properties_to_check.update(device_properties["gestures"])
    if os.path.exists(case.case_property_file):
      case_properties = json.load(open(case.case_property_file))
      if "gestures" in case_properties:
        properties_to_check.update(case_properties["gestures"])

    for key in properties_to_check.keys():
      if key not in original_log.properties:
        # Ignore properties that didn't exist when log was created
        continue
      replay_value = replay_log.properties[key]
      original_value = original_log.properties[key]

      try:
        # try comparing values as floats
        delta = math.fabs(float(replay_value) - float(original_value))
        if delta > 1e-10:
          PrintMismatch("Property", key, original_value, replay_value)
          property_errors = True
      except ValueError:
        # values are not floats. Use direct comparison
        if replay_value != original_value:
          PrintMismatch("Property", key, original_value, replay_value)
          property_errors = True

    # check hardware properties
    hwproperties_errors = False
    for key, original_value in original_log.hwproperties.items():
      if not key in replay_log.hwproperties:
        print("  HwProperty '%s' is missing" % key)
        hwproperties_errors = True

      replay_value = replay_log.hwproperties[key]
      if math.fabs(original_value - replay_value) > 1E-10:
        PrintMismatch("HwProperty", key, original_value, replay_value)
        hwproperties_errors = True

    # check sequence of hwstates and callbacks
    hwstates_errors = False
    def EntryTime(entry):
      if "now" in entry:
        return entry["now"]
      elif "timestamp" in entry:
        return entry["timestamp"]
      elif "endTime" in entry:
        return entry["endTime"]
      else:
        return -1

    # A callback might come in too early. The driver will then reset the timer
    # to get a callback that at least had the desired time passed.
    # We need to remove those as this does not happen with the replay fake
    # timers. Those always get called at the desired time.
    cleaned_original_entries = []
    timer = 0
    for i, entry in enumerate(original_log.raw["entries"]):
      if entry["type"] == "callbackRequest" and i > 0:
        time = EntryTime(original_log.raw["entries"][i - 1])
        timer = time + entry["when"]
      elif entry["type"] == "timerCallback" and timer > 0:
        delta = entry["now"] - timer
        if delta > 0:
          cleaned_original_entries.append(entry)
      elif entry["type"] == "hardwareState":
        cleaned_original_entries.append(entry)

    cleaned_replay_entries = [e for e in replay_log.raw["entries"]
                                if e["type"] == "hardwareState"
                                or e["type"] == "timerCallback"]

    if len(cleaned_original_entries) != len(cleaned_replay_entries):
      hwstates_errors = True
    else:
      for i, replay_entry in enumerate(cleaned_replay_entries):
        if replay_entry["type"] != cleaned_original_entries[i]["type"]:
          hwstates_errors = True
          break

    # validate resulting gestures
    gestures_errors = False
    if len(replay_log.gestures) != len(original_log.gestures):
      gestures_errors = True
    else:
      for i, replay_gesture in enumerate(replay_log.gestures):
        original_gesture = original_log.gestures[i]
        if original_gesture.type != replay_gesture.type:
          gestures_errors = True

    print("Warnings: ")

    if hwproperties_errors:
      print("- Hardware properties do not match.")
      print("  Are you creating the test for the right platform?")

    if property_errors:
      print("- Device properties do not match.")
      print("  This can happen if platform.props is out of date or you are" + \
            " adding an older log.")

    if hwstates_errors:
      print("- The hardware state + timer callback sequence is not the same!")
      print("  This is a technical limitation and might cause different" + \
            " behavior.")

    if gestures_errors:
      print("- Generated gestures do not match.")
      print("  Original gestures:")
      print("    " + "\n    ".join([str(g) for g in original_log.gestures]))
      print("  Replay gestures:")
      print("    " + "\n    ".join([str(g) for g in replay_log.gestures]))
      print("  This might be caused by one of the warnings above or simply")
      print("  because the drivers behavior changed since the log was created.")

    if not(gestures_errors or hwproperties_errors or property_errors
           or hwstates_errors):
      print("  none :)")
