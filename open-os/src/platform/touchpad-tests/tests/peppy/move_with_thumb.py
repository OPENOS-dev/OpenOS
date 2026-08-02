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
#   Motion d=1.787 x=0 y=1.787 r=0.178 s=36.79
#   Motion d=0.4468 x=0 y=0.4468 r=2.776e-17 s=12.15
#   Scroll d=3 x=0 y=3 r=0.2987 s=105.5
#   Motion d=1010 x=572 y=600.3 r=2.025 s=475.1
#   Scroll d=12 x=0 y=12 r=1.776e-15 s=883.8
#   FlingStop
#   Motion d=799.7 x=429.3 y=496.2 r=4.634 s=1422
#   FlingStop
#   Motion d=2352 x=1403 y=1351 r=6.748 s=1470
#   FlingStop
#   Motion d=465 x=347.9 y=238.4 r=3.701 s=703.9
#   FlingStop
#   Motion d=326.1 x=265.9 y=144.2 r=1.315 s=303.9
#   FlingStop
#   Motion d=1348 x=856 y=784.1 r=3.718 s=853.6
#   FlingStop
#   Motion d=1027 x=640.1 y=784.2 r=5.563 s=1102
#   Motion d=193.7 x=96.29 y=161.5 r=3.608 s=546.4
#   Motion d=529.9 x=294.6 y=431.1 r=6.084 s=1036

def Validate(raw, events, gestures):
  fuzzy = FuzzyCheck()
  fuzzy.expected = [
    MotionValidator(">=8200 ~ 500", merge=True),
  ]
  fuzzy.unexpected = [
    ScrollValidator("== 0 ~ 15", merge=True),
    FlingStopValidator("<10"),
  ]
  return fuzzy.Check(gestures)
