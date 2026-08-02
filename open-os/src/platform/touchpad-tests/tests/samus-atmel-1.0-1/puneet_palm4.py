# Copyright 2015 The Chromium OS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

from fuzzy_check import FuzzyCheck
from validators import *

# originally generated gestures:
# FlingStop
#   Motion d=0.5585 x=0 y=0.5585 r=1.11e-16 s=61.12

def Validate(raw, events, gestures):
  fuzzy = FuzzyCheck()
  fuzzy.expected = [
    MotionValidator("== 0 ~ 1"),
    ]
  fuzzy.unexpected = [
    FlingStopValidator("<10"),
    ]
  return fuzzy.Check(gestures)
