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
#   Motion d=26 x=2 y=25 r=0.15
#   Scroll d=14 x=1 y=13 r=0.46
#   Scroll d=1 x=0 y=1 r=0.00

def Validate(raw, events, gestures):
  fuzzy = FuzzyCheck()
  fuzzy.expected = [
    MotionValidator("== 0 ~ 27"),
    ScrollValidator(">= 14"),
  ]
  fuzzy.unexpected = [
    FlingStopValidator("<10"),
    ScrollValidator(),
    ScrollValidator(),
  ]
  return fuzzy.Check(gestures)
