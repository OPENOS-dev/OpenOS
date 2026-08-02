# Copyright (c) 2012 The Chromium OS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

from fuzzy_check import FuzzyCheck
from validators import *

# originally generated gestures:
# Scroll d=212 x=0 y=212 r=4.41
#   Fling d=519 x=0 y=519 r=0.00
#   FlingStop
#   Motion d=3 x=0 y=3 r=0.74
#   Scroll d=552 x=9 y=548 r=5.75
#   Fling d=4147 x=0 y=4147 r=0.00
#   FlingStop
#   FlingStop
#   Motion d=194 x=27 y=189 r=2.55
#   FlingStop
#   Scroll d=544 x=4 y=543 r=4.26
#   Fling d=1417 x=0 y=1417 r=0.00

def Validate(raw, events, gestures):
  fuzzy = FuzzyCheck()
  fuzzy.expected = [
    # expect 4 independent scrolls.
    ScrollValidator(">= 100"),
    FlingValidator(">= 100"),
    ScrollValidator(">= 100"),
    FlingValidator(">= 100"),
    ScrollValidator(">= 100"),
    FlingValidator(">= 100"),
    ScrollValidator(">= 100"),
    FlingValidator(">= 100"),
  ]
  fuzzy.unexpected = [
    MotionValidator("<100"),
    FlingStopValidator("<10"),
  ]
  return fuzzy.Check(gestures)
