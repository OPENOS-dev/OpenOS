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
#   Motion d=26.28 x=2.379 y=26.17 r=0
#   Scroll d=2185 x=0 y=2185 r=24.19
#   Fling d=5548 x=0 y=5548 r=9.095e-13
#   FlingStop

def Validate(raw, events, gestures):
  fuzzy = FuzzyCheck()
  fuzzy.expected = [
    ScrollValidator(">10"),
  ]
  fuzzy.unexpected = [
    MotionValidator("==0 ~ 100"),
    FlingStopValidator("<10"),
    FlingValidator(),
  ]
  return fuzzy.Check(gestures)
