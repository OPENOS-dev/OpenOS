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
#   FlingStop
#   ButtonDown(1)
#   Motion d=2 x=0 y=1 r=0.00
#   ButtonUp(1)
#   FlingStop
#   FlingStop
#   ButtonDown(1)
#   Motion d=1 x=1 y=0 r=0.05
#   ButtonUp(1)

def Validate(raw, events, gestures):
  fuzzy = FuzzyCheck()
  fuzzy.expected = [
    ButtonDownValidator(4),
    ButtonUpValidator(4),
    ButtonDownValidator(4),
    ButtonUpValidator(4),
  ]
  fuzzy.unexpected = [
    MotionValidator("<10", merge=True),
    FlingStopValidator("<10"),
  ]
  return fuzzy.Check(gestures)
