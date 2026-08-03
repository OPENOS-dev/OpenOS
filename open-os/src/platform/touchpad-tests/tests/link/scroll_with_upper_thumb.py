# Copyright (c) 2013 The Chromium OS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

from fuzzy_check import FuzzyCheck
from validators import *

# originally generated gestures:
# FlingStop
#   Scroll d=161 x=0 y=161 r=1.52
#   Scroll d=0 x=0 y=0 r=0
#   Scroll d=113 x=0 y=113 r=2.202
#   Scroll d=1 x=0 y=1 r=0
#   Scroll d=0 x=0 y=0 r=0
#   Fling d=0 x=0 y=0 r=0
#   Motion d=8.522e-06 x=0 y=8.522e-06 r=1.694e-21
#   FlingStop
#   Scroll d=56 x=18 y=38 r=0.3951
#   Fling d=0 x=0 y=0 r=0
#   Motion d=0.4468 x=0 y=0.4468 r=5.551e-17

def Validate(raw, events, gestures):
  fuzzy = FuzzyCheck()
  fuzzy.expected = [
    ScrollValidator(">= 100"),
    ScrollValidator(">= 100", merge=True),
  ]
  fuzzy.unexpected = [
    MotionValidator("<1", merge=True),
    FlingValidator(),
    FlingStopValidator("<10"),
    # This scroll should (maybe) be a move,
    # but we get it wrong currently in the
    # gesture recognizer.
    ScrollValidator("< 65"),
  ]
  return fuzzy.Check(gestures)

# this behavior is currently not supported by the gesture recognizer
Validate.disabled = True
