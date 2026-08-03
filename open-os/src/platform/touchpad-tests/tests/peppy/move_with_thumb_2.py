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
#   Motion d=1726 x=1469 y=787.4 r=8.685 s=2374
#   FlingStop
#   Motion d=926.5 x=887.2 y=231.2 r=11.51 s=3231
#   Motion d=1406 x=818.9 y=898.4 r=6.272 s=1693
#   Scroll d=337.2 x=296 y=79 r=7.763 s=2077
#   Scroll d=147 x=147 y=0 r=7.136 s=1676
#   FlingStop
#   Scroll d=578.8 x=118 y=498 r=14.83 s=6510
#   Motion d=84.78 x=56.98 y=51.87 r=4.482 s=1112
#   FlingStop
#   FlingStop
#   Motion d=693.7 x=601.3 y=188.7 r=12.44 s=4337
#   Scroll d=686 x=0 y=686 r=11.21 s=5612
#   Motion d=478 x=375.6 y=187.3 r=9.371 s=1595
#   Scroll d=782.9 x=410 y=424 r=12.93 s=3207
#   Motion d=627.7 x=535.7 y=177.5 r=5.557 s=2202
#   Scroll d=1.247e+04 x=1.128e+04 y=1567 r=20.74 s=3509
#   Motion d=264.6 x=242.8 y=39.47 r=7.611 s=2985
#   Motion d=5.169 x=4.903 y=1.634 r=0 s=478.3

def Validate(raw, events, gestures):
  fuzzy = FuzzyCheck()
  fuzzy.expected = [
    MotionValidator(">=15800 ~ 500", merge=True),
  ]
  fuzzy.unexpected = [
    FlingStopValidator("<10"),
  ]
  return fuzzy.Check(gestures)
