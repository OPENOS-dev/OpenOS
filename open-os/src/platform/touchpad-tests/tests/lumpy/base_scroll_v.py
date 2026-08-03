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
#   Scroll d=1637 x=1637 y=0 r=5.32
#   Fling d=498 x=498 y=0 r=0.00
#   Scroll d=1775 x=1775 y=0 r=8.42
#   Fling d=1806 x=1806 y=0 r=0.00

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
    FlingStopValidator("<10"),
    FlingValidator()
  ]
  return fuzzy.Check(gestures)
