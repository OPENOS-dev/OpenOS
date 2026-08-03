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
#   Motion d=1 x=0 y=1 r=0.00
#   FlingStop
#   Scroll d=955 x=0 y=955 r=3.40
#   Fling d=952 x=0 y=952 r=0.00
#   FlingStop
#   Scroll d=1114 x=0 y=1114 r=6.85

def Validate(raw, events, gestures):
  fuzzy = FuzzyCheck()
  fuzzy.expected = [
    AnythingButValidator(ScrollValidator()),
    ScrollValidator(),
    AnythingButValidator(ScrollValidator()),
    ScrollValidator()
  ]
  fuzzy.unexpected = [
    MotionValidator("<10"),
    FlingStopValidator("<10")
  ]
  return fuzzy.Check(gestures)
