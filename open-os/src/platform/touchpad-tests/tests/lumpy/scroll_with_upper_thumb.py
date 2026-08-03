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
#   Scroll d=233 x=0 y=233 r=5.24
#   FlingStop
#   Motion d=68.3 x=5.497 y=66.69 r=2.242
#   FlingStop
#   Motion d=37.27 x=1.048 y=37.22 r=1.608

def Validate(raw, events, gestures):
  fuzzy = FuzzyCheck()
  fuzzy.expected = [
    ScrollValidator(">= 100"),
    FlingValidator(">= 100"),
    ScrollValidator(">= 100"),
    FlingValidator(">= 100"),
  ]
  fuzzy.unexpected = [
    MotionValidator("<30", merge=True),
    FlingStopValidator("<10"),
  ]
  return fuzzy.Check(gestures)
