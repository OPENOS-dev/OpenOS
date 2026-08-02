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
#   Motion d=1 x=0 y=1 r=0.00
#   Motion d=1 x=0 y=1 r=0.00
#   Motion d=1 x=0 y=1 r=0.07
#   Motion d=0 x=0 y=0 r=0.00
#   Motion d=2 x=0 y=2 r=0.37
#   Motion d=3 x=0 y=3 r=0.39
#   Motion d=2 x=0 y=2 r=0.37
#   Motion d=2 x=0 y=2 r=0.18
#   Motion d=0 x=0 y=0 r=0.00

def Validate(raw, events, gestures):
  fuzzy = FuzzyCheck()
  fuzzy.expected = [
    MotionValidator("<1", merge=True),
  ]
  fuzzy.unexpected = [
    FlingStopValidator("<10"),
  ]
  return fuzzy.Check(gestures)
