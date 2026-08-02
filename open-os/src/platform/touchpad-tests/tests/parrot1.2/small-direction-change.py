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
#   Motion d=37 x=17 y=23 r=0.15
#   Motion d=4 x=3 y=2 r=0.14

# Direction change with small delta was marked as WARP which causes
# box filter to reset box center and occasionally  stops some following
# movement from pushing the box, which results in no cursor movement
# We have a fix to not mark WARP for direction change with small delta,
# and this test ensures the expected movement to happen.

def Validate(raw, events, gestures):
  fuzzy = FuzzyCheck()
  fuzzy.expected = [
    FlingStopValidator(),
    MotionValidator(">= 37 ~ 6"),
  ]
  fuzzy.unexpected = [
    # FlingStopValidator("<10"),
  ]
  return fuzzy.Check(gestures)
