# Copyright (c) 2014 The Chromium OS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

from fuzzy_check import FuzzyCheck
from validators import *
from mtlib.gesture_log import MotionGesture

def Validate(raw, events, gestures, original_values=None):
  fuzzy = FuzzyCheck()
  # Make sure the values always match the original ones we recorded
  fuzzy.expected = [
    MotionValidator("== %d ~ %d" %
                    (original_values[0], original_values[0] * 0.2)),
    MotionValidator("== %d ~ %d" %
                    (original_values[1], original_values[1] * 0.2)),
    MotionValidator("== %d ~ %d" %
                    (original_values[2], original_values[2] * 0.2)),
    MotionValidator("== %d ~ %d" %
                    (original_values[3], original_values[3] * 0.2)),
  ]
  fuzzy.unexpected = [
    FlingStopValidator("<10"),
  ]
  return fuzzy.Check(gestures)

def GenerateOriginalValues(raw, events, gestures):
  # Given originally collected gestures, generate the "original_values"
  # object that will be passed in above. This must be JSON-serializable.
  # Also does a sanity check to make sure right gesture was performed.
  # Returns (success, error_message, original_values).

  # Sanity check
  fuzzy = FuzzyCheck()
  fuzzy.expected = [
    MotionValidator("> 200 ~ 200"),
    MotionValidator("> 200 ~ 200"),
    MotionValidator("> 200 ~ 200"),
    MotionValidator("> 200 ~ 200"),
  ]
  fuzzy.unexpected = [
    FlingStopValidator("<10"),
  ]
  ret = fuzzy.Check(gestures)

  if ret[0]:
    # Sanity check passed; generate original values
    original_values = []
    for gesture in gestures:
      if isinstance(gesture, MotionGesture):
        original_values.append(gesture.Distance())
    if len(original_values) != 4:
      return (False, "Incorrect number of moves", None)
    ret = ret + tuple([original_values])
  else:
    ret = ret + (None,)
  return ret

def UserInstructions():
  return "Using 1 finger, swipe in these directions: up, right, down, left"

def InstructRobot(robot):
  robot.Line((0.5, 0.1, -45, 20), (0.5, 0.5, 0, 20), (1, 0, 0, 0), 70, "basic")
  robot.Line((0.5, 0.5, -45, 20), (0.5, 0.1, 0, 20), (1, 0, 0, 0), 70, "basic")
  robot.Line((0.1, 0.5, -45, 20), (0.9, 0.5, 0, 20), (1, 0, 0, 0), 70, "basic")
  robot.Line((0.9, 0.5, -45, 20), (0.1, 0.5, 0, 20), (1, 0, 0, 0), 70, "basic")
