# Copyright (c) 2014 The Chromium OS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

from fuzzy_check import FuzzyCheck
from validators import *
from mtlib.gesture_log import ButtonDownGesture
from mtlib.gesture_log import MotionGesture

def Validate(raw, events, gestures, original_values=None):
  fuzzy = FuzzyCheck()
  # Make sure the values always match the original ones we recorded
  fuzzy.expected = [
    ButtonDownValidator(1),
    MotionValidator("== %d ~ %d" %
                    (original_values, original_values * 0.2)),
    ButtonUpValidator(1),
  ]
  fuzzy.unexpected = [
    MotionValidator("<10", merge=True),
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
    ButtonDownValidator(1),
    MotionValidator("> 200"),
    ButtonUpValidator(1),
  ]
  fuzzy.unexpected = [
    MotionValidator("<10", merge=True),
    FlingStopValidator("<10"),
  ]
  ret = fuzzy.Check(gestures)

  if ret[0]:
    # Sanity check passed; generate original values
    original_values = 0.0
    got_button_down = False
    for gesture in gestures:
      if isinstance(gesture, ButtonDownGesture):
        got_button_down = True
        continue
      if isinstance(gesture, MotionGesture) and got_button_down:
        original_values = gesture.Distance()
    if original_values == 0.0:
      return (False, "Can't find Motion distance", None)
    ret = ret + (original_values,)
  else:
    ret = ret + (None,)
  return ret

def UserInstructions():
  return "Pressing with thumb and moving with finger, do a click drag"
