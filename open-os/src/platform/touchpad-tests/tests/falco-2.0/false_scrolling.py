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
#   Scroll d=308.4 x=1 y=308 r=12.3 s=1021
#   Scroll d=1 x=0 y=1 r=0 s=72.05
#   Scroll d=325.4 x=1 y=325 r=26.31 s=2119
#   Scroll d=1598 x=0 y=1598 r=16.15 s=2408
#   Scroll d=1598 x=0 y=1598 r=20.48 s=1343
#   Scroll d=1187 x=3 y=1185 r=21.42 s=2382
#   Scroll d=921.4 x=2 y=920 r=20.75 s=1516
#   Scroll d=1590 x=17 y=1574 r=23.03 s=1617
#   Fling d=0 x=0 y=0 r=0 s=0

def Validate(raw, events, gestures):
  fuzzy = FuzzyCheck()
  fuzzy.expected = [
    ScrollValidator("< 208~50", merge=True),
  ]
  fuzzy.unexpected = [
    # MotionValidator("<10"),
    FlingStopValidator("<10"),
    FlingValidator(),
  ]
  return fuzzy.Check(gestures)
