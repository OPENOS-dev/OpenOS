# Copyright 2015 The Chromium OS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

from fuzzy_check import FuzzyCheck
from validators import *

# originally generated gestures:
# FlingStop
#   Motion d=7.09 x=1.009 y=6.774 r=0.02345 s=27.09
#   Motion d=0.1441 x=0 y=0.1441 r=2.776e-17 s=15.2
#   Motion d=160.3 x=38.22 y=152.8 r=0.3225 s=57.64
#   Scroll d=27.36 x=0 y=27.36 r=0.2106 s=141.1
#   Scroll d=3.04 x=1.52 y=1.52 r=0.2158 s=78.09
#   Fling d=0 x=0 y=0 r=0 s=0

def Validate(raw, events, gestures):
  fuzzy = FuzzyCheck()
  fuzzy.expected = [
    ScrollValidator("== 392.5 ~ 8.3", merge=True),
  ]
  fuzzy.unexpected = [
    MotionValidator("== 0"),
    FlingStopValidator(),
    FlingValidator()
  ]
  return fuzzy.Check(gestures)
