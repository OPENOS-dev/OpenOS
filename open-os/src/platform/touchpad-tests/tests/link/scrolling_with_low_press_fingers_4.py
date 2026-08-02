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
#   Motion d=24 x=0 y=24 r=0.64
#   Scroll d=61 x=0 y=61 r=0.40
#   Fling d=55 x=1 y=55 r=0.00

def Validate(raw, events, gestures):
  fuzzy = FuzzyCheck()
  fuzzy.expected = [
    MotionValidator("== 0 ~ 25"),
    ScrollValidator(">= 61"),
  ]
  fuzzy.unexpected = [
    FlingStopValidator("<10"),
    FlingValidator(),
  ]
  return fuzzy.Check(gestures)
