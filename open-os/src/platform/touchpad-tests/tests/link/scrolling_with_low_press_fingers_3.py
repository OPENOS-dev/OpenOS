# Copyright (c) 2012 The Chromium OS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

from fuzzy_check import FuzzyCheck
from validators import *

# originally generated gestures:
# Motion d=27 x=0 y=27 r=0.68
#   FlingStop
#   Scroll d=33 x=0 y=33 r=0.43
#   Scroll d=1 x=0 y=1 r=0.30
#   Fling d=0 x=0 y=0 r=0.00

def Validate(raw, events, gestures):
  fuzzy = FuzzyCheck()
  fuzzy.expected = [
    MotionValidator("== 0 ~ 28"),
    ScrollValidator(">= 33"),
  ]
  fuzzy.unexpected = [
    FlingValidator(),
    FlingStopValidator("<10"),
    ScrollValidator(),
  ]
  return fuzzy.Check(gestures)
