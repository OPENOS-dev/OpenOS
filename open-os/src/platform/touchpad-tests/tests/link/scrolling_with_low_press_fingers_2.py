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
#   Motion d=18 x=0 y=18 r=1.34
#   Scroll d=47 x=0 y=47 r=0.36
#   Fling d=34 x=0 y=34 r=0.00

def Validate(raw, events, gestures):
  fuzzy = FuzzyCheck()
  fuzzy.expected = [
    MotionValidator("== 0 ~ 19"),
    ScrollValidator(">= 47"),
  ]
  fuzzy.unexpected = [
    FlingStopValidator("<10"),
    FlingValidator(),
  ]
  return fuzzy.Check(gestures)
