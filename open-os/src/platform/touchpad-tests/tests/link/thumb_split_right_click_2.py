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
#   FlingStop
#   Motion d=13.4 x=13.4 y=0 r=0.3163 s=171.5
#   Motion d=595.4 x=576.4 y=76.06 r=3.116 s=1287
#   FlingStop
#   ButtonDown(4)
#   ButtonUp(4)
#   Motion d=1.787 x=1.787 y=0 r=2.22e-16 s=274.2

def Validate(raw, events, gestures):
  fuzzy = FuzzyCheck()
  fuzzy.expected = [
    ButtonDownValidator(1),
    ButtonUpValidator(1),
  ]
  fuzzy.unexpected = [
    MotionValidator(merge=True),
    FlingStopValidator("<10"),
  ]
  return fuzzy.Check(gestures)
