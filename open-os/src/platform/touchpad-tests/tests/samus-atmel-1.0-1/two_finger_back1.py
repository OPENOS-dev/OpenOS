# Copyright 2015 The Chromium OS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

from fuzzy_check import FuzzyCheck
from validators import *

# originally generated gestures:
# Scroll d=278.1 x=277.6 y=0.9818 r=1.623 s=971.5
#   FlingStop
#   Motion d=119.8 x=102.8 y=61.22 r=3.163 s=4434

def Validate(raw, events, gestures):
  fuzzy = FuzzyCheck()
  fuzzy.expected = [
     ScrollValidator("== 231.8 ~25.0"),
     FlingValidator("== 0 ~ 10"),
     FlingStopValidator(),
     MotionValidator("== 119.8 ~ 20"),
  ]
  fuzzy.unexpected = [
     FlingStopValidator("<10"),
  ]
  return fuzzy.Check(gestures)
