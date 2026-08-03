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
#   Motion d=59 x=55 y=17 r=0.01
#   FlingStop
#   ButtonDown(1)
#   ButtonUp(1)
#   FlingStop
#   Motion d=113 x=89 y=67 r=0.04
#   FlingStop
#   ButtonDown(1)
#   ButtonUp(1)
#   Motion d=123 x=5 y=122 r=0.16
#   FlingStop
#   FlingStop
#   ButtonDown(1)
#   ButtonUp(1)
#   FlingStop
#   Motion d=140 x=85 y=109 r=0.10
#   FlingStop
#   ButtonDown(1)
#   ButtonUp(1)
#   FlingStop
#   Motion d=156 x=66 y=140 r=0.08
#   FlingStop
#   ButtonDown(1)
#   ButtonUp(1)
#   FlingStop
#   Motion d=86 x=57 y=64 r=0.31

def Validate(raw, events, gestures):
  fuzzy = FuzzyCheck()
  fuzzy.expected = [
    ButtonDownValidator(1),
    MotionValidator("> 100 ~ 50"),
    ButtonUpValidator(1),
  ]
  fuzzy.unexpected = [
    MotionValidator("<100"),
    FlingStopValidator("<10"),
  ]
  return fuzzy.Check(gestures)
