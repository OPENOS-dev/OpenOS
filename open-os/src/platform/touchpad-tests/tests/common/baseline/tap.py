# Copyright (c) 2014 The Chromium OS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

from fuzzy_check import FuzzyCheck
from validators import *

def Validate(raw, events, gestures):
  fuzzy = FuzzyCheck()
  fuzzy.expected = [
    ButtonDownValidator(1),
    ButtonUpValidator(1),
    ButtonDownValidator(4),
    ButtonUpValidator(4)
  ]
  fuzzy.unexpected = [
    FlingStopValidator("<10"),
  ]
  return fuzzy.Check(gestures)

def UserInstructions():
  return "Do a 1 finger tap, then a 2-finger tap"

def InstructRobot(robot):
  robot.Tap((0.5, 0.2, -45, 20), (1, 0, 0, 0), 1, 0.1)
  robot.Tap((0.5, 0.2, -45, 20), (1, 0, 1, 0), 1, 0.0)

