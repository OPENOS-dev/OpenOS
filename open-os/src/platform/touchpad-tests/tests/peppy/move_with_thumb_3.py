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
#   FlingStop
#   Motion d=468.5 x=390.3 y=170.8 r=8.241 s=892.1
#   Scroll d=52.8 x=30 y=30 r=2.431 s=991.5
#   Scroll d=356.4 x=301 y=61 r=21.04 s=2685
#   Motion d=3455 x=2903 y=1177 r=13.96 s=2955
#   FlingStop
#   Motion d=1.803e+04 x=1.345e+04 y=8270 r=15.95 s=3080
#   FlingStop
#   FlingStop
#   FlingStop
#   Scroll d=161 x=0 y=161 r=6.359 s=2145
#   FlingStop
#   Scroll d=676 x=632 y=44 r=38.68 s=7844
#   Scroll d=440.7 x=28 y=427 r=9.596 s=2694
#   Motion d=111.1 x=73.16 y=51.81 r=7.641 s=523

def Validate(raw, events, gestures):
  fuzzy = FuzzyCheck()
  fuzzy.expected = [
    MotionValidator(">=22900 ~ 500", merge=True),
  ]
  fuzzy.unexpected = [
    ScrollValidator("== 0 ~ 500", merge=True),
    FlingStopValidator("<10"),
  ]
  return fuzzy.Check(gestures)
