# Copyright (c) 2013 The Chromium OS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

from fuzzy_check import FuzzyCheck
from validators import *

# originally generated gestures:
# Motion d=4.44 x=4.258 y=0.4804 r=0.1059 s=52.85
#   Motion d=123.9 x=119.8 y=15.2 r=0.7423 s=95.05
#   FlingStop
#   Scroll d=127 x=0 y=127 r=10.45 s=1323
#   Motion d=0.1571 x=0 y=0.1571 r=0.01268 s=0.9131

def Validate(raw, events, gestures):
  fuzzy = FuzzyCheck()
  fuzzy.expected = [
    ScrollValidator("> 583.3", merge=True),
    FlingValidator(">4166.7"),
  ]
  fuzzy.unexpected = [
    FlingStopValidator("<10"),
    MotionValidator(merge=True),
  ]
  return fuzzy.Check(gestures)
