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
#   Scroll d=48 x=0 y=48 r=1.41
#   Motion d=5 x=1 y=4 r=0.00
#   Scroll d=1843 x=4 y=1839 r=24.59
#   Fling d=26016 x=0 y=26016 r=0.00

def Validate(raw, events, gestures):
  fuzzy = FuzzyCheck()
  fuzzy.expected = [
    ScrollValidator("> 1500.0 ~ 833.3", merge=True),
    FlingValidator("> 16666.7 ~ 8333.3"),
  ]
  fuzzy.unexpected = [
    MotionValidator("== 0 ~ 10", merge=True),
    FlingStopValidator("<10"),
  ]
  return fuzzy.Check(gestures)
