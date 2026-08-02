# Copyright (c) 2012 The Chromium OS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

from fuzzy_check import FuzzyCheck
from validators import *

# originally generated gestures:
# Motion d=81 x=77 y=21 r=1.29
#   FlingStop
#   FlingStop
#   Motion d=1 x=0 y=1 r=0.00
#   ButtonDown(1)
#   ButtonUp(1)

# This test looks for the incorrect tap-click to not occur.

def Validate(raw, events, gestures):
  fuzzy = FuzzyCheck()
  fuzzy.expected = [
    MotionValidator("> 0", merge=True),
    # Tap-click not expected here
  ]
  fuzzy.unexpected = [
    FlingStopValidator("<10"),
  ]
  return fuzzy.Check(gestures)
