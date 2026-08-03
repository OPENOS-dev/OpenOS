# Copyright (c) 2012 The Chromium OS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

from fuzzy_check import FuzzyCheck
from validators import *

# originally generated gestures:
# FlingStop
#   Scroll d=3851 x=14 y=3837 r=81.87
#   Fling d=78095 x=0 y=78095 r=0.00
#   FlingStop
#   Scroll d=39 x=39 y=0 r=0.19
#   Fling d=750 x=750 y=0 r=0.00
#   FlingStop

def Validate(raw, events, gestures):
  fuzzy = FuzzyCheck()
  fuzzy.expected = [
    FlingValidator(">0"),
    FlingStopValidator(">=1"),
    FlingValidator(">0"),
    FlingStopValidator(">=1"),
  ]
  fuzzy.unexpected = [
    MotionValidator("<10", merge=True),
    FlingStopValidator("<10"),
    ScrollValidator(merge=True)
  ]
  return fuzzy.Check(gestures)
