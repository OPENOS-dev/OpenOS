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
#   Motion d=186.2 x=69.37 y=171.1 r=4.278 s=393.5
#   FlingStop
#   ButtonDown(1)
#   Motion d=185.6 x=70.38 y=158.7 r=3.193 s=290.9
#   ButtonUp(1)
#   Scroll d=3 x=0 y=3 r=4.441e-16 s=192
#   Fling d=0 x=0 y=0 r=0 s=0

def Validate(raw, events, gestures):
  fuzzy = FuzzyCheck()
  fuzzy.expected = [
    MotionValidator(),
    ButtonDownValidator(1),
    MotionValidator(),
    ButtonUpValidator(1),
  ]
  fuzzy.unexpected = [
    FlingStopValidator("<10"),
  ]
  return fuzzy.Check(gestures)
