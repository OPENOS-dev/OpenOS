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
#   Motion d=27 x=3 y=27 r=0.77
#   Scroll d=234 x=0 y=234 r=0.64

def Validate(raw, events, gestures):
  fuzzy = FuzzyCheck()
  fuzzy.expected = [
    ScrollValidator("== 234 ~ 100"),
    FlingValidator(),
  ]
  fuzzy.unexpected = [
    MotionValidator("== 0 ~ 1000"),
    FlingStopValidator("<10"),
  ]
  return fuzzy.Check(gestures)
