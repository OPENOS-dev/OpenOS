# Copyright (c) 2014 The Chromium OS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

from fuzzy_check import FuzzyCheck
from validators import *
from mtlib.gesture_log import FlingGesture
from mtlib.gesture_log import ScrollGesture

def Validate(raw, events, gestures, original_values=None):
  fuzzy = FuzzyCheck()
  fuzzy.expected = [
    FlingStopValidator(">=0"),

    # scroll and fling
    ScrollValidator("== %d ~ %d" % (original_values['scroll'][0],
                                    original_values['scroll'][0] * 0.2)),
    FlingValidator("== %d ~ %d" % (original_values['fling'][0],
                                   original_values['fling'][0] * 0.2)),

    # fling stop and scroll
    FlingStopValidator(">=1"),
    ScrollValidator("== %d ~ %d" % (original_values['scroll'][1],
                                    original_values['scroll'][1] * 0.2)),
    FlingValidator("== %d ~ %d" % (original_values['fling'][1],
                                   original_values['fling'][1] * 0.2)),
  ]
  fuzzy.unexpected = [
  ]
  return fuzzy.Check(gestures)

def GenerateOriginalValues(raw, events, gestures):
  fuzzy = FuzzyCheck()
  fuzzy.expected = [
    FlingStopValidator(">=0"),

    # scroll and fling
    ScrollValidator("> 0"),
    FlingValidator("> 0"),

    # fling stop and scroll
    FlingStopValidator(">=1"),
    ScrollValidator("> 0", merge=True),
    FlingValidator(">= 0")
  ]
  fuzzy.unexpected = [
  ]
  ret = fuzzy.Check(gestures)

  if ret[0]:
    # Sanity check passed; generate original values
    original_values = {"scroll": [], "fling": []};
    for gesture in gestures:
      if isinstance(gesture, ScrollGesture):
        original_values['scroll'].append(gesture.Distance())
      if isinstance(gesture, FlingGesture):
        original_values['fling'].append(gesture.Distance())
    if (len(original_values['scroll']) == 0 or
        len(original_values['fling']) == 0):
      return (False, "Can't find scroll and/or fling values", None)
    ret = ret + tuple([original_values])
  else:
    ret = ret + (None,)
  return ret

def UserInstructions():
  return ("Scroll and fling from top to bottom, then do another one.\n" +
          "Pause after starting the second one to make a fling-stop event")

def InstructRobot(robot):
  # scroll and fling
  robot.Line((0.5, 0.0, -45, 20), (0.5, 0.5, -45, 20),
             (1, 0, 1, 0), 30, "fling", 0.5)
  # fling stop and scroll a little
  robot.Line((0.5, 0.5, -45, 20), (0.5, 0.4, -45, 20), (1, 0, 1, 0), 1, "basic")
