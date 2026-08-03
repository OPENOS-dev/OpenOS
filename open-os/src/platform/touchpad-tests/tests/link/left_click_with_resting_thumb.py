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
#   Motion d=12 x=0 y=12 r=0.15
#   FlingStop
#   ButtonDown(1)
#   Motion d=1 x=0 y=1 r=0.01
#   ButtonUp(1)
#   FlingStop
#   ButtonDown(4)
#   Motion d=1 x=0 y=0 r=0.01
#   ButtonUp(4)

def Validate(raw, events, gestures):
  fuzzy = FuzzyCheck()
  fuzzy.expected = [
    ButtonDownValidator(1),
    ButtonUpValidator(1),
    ButtonDownValidator(1),
    ButtonUpValidator(1),
  ]
  fuzzy.unexpected = [
    MotionValidator("<100", merge=True),
    FlingStopValidator("<10"),
  ]
  return fuzzy.Check(gestures)
