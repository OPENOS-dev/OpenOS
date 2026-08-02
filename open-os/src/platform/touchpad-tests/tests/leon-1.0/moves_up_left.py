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
#   Motion d=343.1 x=271.2 y=151.7 r=8.769 s=1567
#   FlingStop
#   Motion d=207.7 x=119.2 y=123.4 r=5.925 s=1113
#   FlingStop
#   Motion d=282.8 x=173.7 y=185.2 r=7.79 s=1526

def Validate(raw, events, gestures):
  fuzzy = FuzzyCheck()
  fuzzy.expected = [
    MotionValidator("== 1581 ~ 50", merge=True),
  ]
  fuzzy.unexpected = [
    FlingStopValidator("<10"),
  ]
  return fuzzy.Check(gestures)
