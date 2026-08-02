# Copyright (c) 2014 The Chromium OS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

from fuzzy_check import FuzzyCheck
from validators import *
from mtlib.gesture_log import ScrollGesture

def Validate(raw, events, gestures, original_values=None):
  fuzzy = FuzzyCheck()
  # Make sure the values always match the original ones we recorded
  fuzzy.expected = [
    ScrollValidator("== %d ~ %d" %
                    (original_values[0], original_values[0] * 0.2)),
    ScrollValidator("== %d ~ %d" %
                    (original_values[1], original_values[1] * 0.2)),
    ScrollValidator("== %d ~ %d" %
                    (original_values[2], original_values[2] * 0.2)),
    ScrollValidator("== %d ~ %d" %
                    (original_values[3], original_values[3] * 0.2)),
  ]
  fuzzy.unexpected = [
    FlingValidator(">=0", merge=True),
    MotionValidator("<10", merge=True),
    FlingStopValidator("<10"),
  ]
  return fuzzy.Check(gestures)

def GenerateOriginalValues(raw, events, gestures):
  # Sanity check
  fuzzy = FuzzyCheck()
  fuzzy.expected = [
    ScrollValidator("> 50 ~ 50"),
    ScrollValidator("> 50 ~ 50"),
    ScrollValidator("> 200 ~ 200"),
    ScrollValidator("> 200 ~ 200"),
  ]
  fuzzy.unexpected = [
    FlingValidator(">=0", merge=True),
    MotionValidator("<10", merge=True),
    FlingStopValidator("<20"),
  ]
  ret = fuzzy.Check(gestures)

  if ret[0]:
    # Sanity check passed; generate original values
    original_values = []
    for gesture in gestures:
      if isinstance(gesture, ScrollGesture):
        original_values.append(gesture.Distance())
    if len(original_values) != 4:
      return (False, "Incorrect number of scrolls", None)
    ret = ret + tuple([original_values])
  else:
    ret = ret + (None,)
  return ret

def UserInstructions():
  return "Using 2 fingers, scroll in these directions: up, right, down, left"

def InstructRobot(robot):
  robot.Line((0.5, 0.1, -45, 20), (0.5, 0.5, -45, 20),
             (1, 0, 1, 0), 70, "basic", 0.5)
  robot.Line((0.5, 0.5, -45, 20), (0.5, 0.1, -45, 20),
             (1, 0, 1, 0), 70, "basic", 0.5)
  robot.Line((0.2, 0.3, -45, 20), (0.8, 0.3, -45, 20),
             (1, 0, 1, 0), 70, "basic", 0.5)
  robot.Line((0.8, 0.3, -45, 20), (0.2, 0.3, -45, 20),
             (1, 0, 1, 0), 70, "basic", 0.5)
