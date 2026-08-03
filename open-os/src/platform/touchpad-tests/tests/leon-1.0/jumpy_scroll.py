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
#   Scroll d=272.5 x=4 y=271 r=22.06 s=540.4
#   Scroll d=14 x=4 y=10 r=0.295 s=39.63
#   Scroll d=341.7 x=3 y=340 r=16.51 s=618.9
#   Scroll d=21.24 x=2 y=20 r=0.6149 s=61.53
#   Scroll d=71 x=0 y=71 r=2.756 s=1308
#   Fling d=827.5 x=0 y=827.5 r=0 s=8.325e+04

def Validate(raw, events, gestures):
  fuzzy = FuzzyCheck()
  fuzzy.expected = [
    ScrollValidator("= 150.0 ~ 25.0", merge=True),
    FlingValidator("== 0")
  ]
  fuzzy.unexpected = [
    FlingStopValidator("<10"),
  ]
  return fuzzy.Check(gestures)
