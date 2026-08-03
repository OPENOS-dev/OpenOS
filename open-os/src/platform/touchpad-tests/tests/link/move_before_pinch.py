# Copyright 2016 The Chromium OS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

from fuzzy_check import FuzzyCheck
from validators import *

# originally generated gestures:
# FlingStop
#   Motion d=7.1 x=3.128 y=6.256 r=0.1597 s=60.57
#   Pinch dz=177.5 r=1.45
#   Motion d=6.948 x=2.681 y=6.256 r=0.2661 s=140.1
#   FlingStop

def Validate(raw, events, gestures):
  fuzzy = FuzzyCheck()
  fuzzy.expected = [
    MotionValidator("== 0 ~ 20"),
    PinchValidator(">= 2.0 ~ 0.5"),
  ]
  fuzzy.unexpected = [
    FlingStopValidator("<10"),
    PinchValidator(),
  ]
  return fuzzy.Check(gestures)
