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
#   ButtonDown(1)
#   Motion d=3241 x=2624 y=1872 r=6.61
#   ButtonUp(1)

def Validate(raw, events, gestures):
  fuzzy = FuzzyCheck()
  fuzzy.expected = [
    ButtonDownValidator(1),
    MotionValidator("== 3241 ~ 2000"),
    ButtonUpValidator(1),
  ]
  fuzzy.unexpected = [
    MotionValidator("== 0 ~ 10"),
    FlingStopValidator("<10"),
  ]
  return fuzzy.Check(gestures)
