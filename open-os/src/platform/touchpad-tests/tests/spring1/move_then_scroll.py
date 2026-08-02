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
#   Motion d=214.7 x=23.05 y=211.9 r=1.05
#   Scroll d=36.83 x=6 y=32 r=0.4727
#   Motion d=108.7 x=13.67 y=106.2 r=0.3244

def Validate(raw, events, gestures):
  fuzzy = FuzzyCheck()
  fuzzy.expected = [
    MotionValidator("== 323 ~ 50"),
    ScrollValidator("== 330 ~ 50"),
  ]
  fuzzy.unexpected = [
    FlingValidator(),
    FlingStopValidator("<10"),
  ]
  return fuzzy.Check(gestures)
