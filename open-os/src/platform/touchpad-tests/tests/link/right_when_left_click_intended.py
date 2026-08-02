# Copyright 2012 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

from fuzzy_check import FuzzyCheck
from validators import *

# originally generated gestures:
# FlingStop
#   Motion d=801 x=191 y=764 r=6.26
#   FlingStop
#   ButtonDown(1)
#   ButtonUp(1)

def Validate(raw, events, gestures):
  fuzzy = FuzzyCheck()
  fuzzy.expected = [
    MotionValidator("<= 970 ~ 50", merge=True),
    ButtonDownValidator(1),
    ButtonUpValidator(1)
  ]
  fuzzy.unexpected = [
    FlingStopValidator("<10"),
    MotionValidator("<1")
  ]
  return fuzzy.Check(gestures)
