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
#   Scroll d=37 x=0 y=37 r=0.80
#   Scroll d=8 x=0 y=8 r=0.25
#   Fling d=194 x=0 y=194 r=0.00
#   FlingStop
#   FlingStop
#   FlingStop
#   Scroll d=79 x=0 y=79 r=0.99
#   Fling d=310 x=0 y=310 r=0.00

def Validate(raw, events, gestures):
  fuzzy = FuzzyCheck()
  fuzzy.expected = [
    # The important thing about this test is that we do not get clicks
  ]
  fuzzy.unexpected = [
    MotionValidator("== 0 ~ 10", merge=True),
    ScrollValidator("<150", merge=True),
    FlingStopValidator("<10"),
    FlingValidator("<1000", merge=True),
  ]
  return fuzzy.Check(gestures)
