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
#   Swipe d=440 x=440 y=0 r=1.19
#   FlingStop
#   Swipe d=376 x=376 y=0 r=1.30
#   FlingStop
#   Swipe d=386 x=0 y=386 r=0.89
#   Swipe d=385 x=0 y=385 r=1.22
#   FlingStop

def Validate(raw, events, gestures):
  fuzzy = FuzzyCheck()
  fuzzy.expected = [
    SwipeValidator(x="==1695 ~ 400"),
    SwipeLiftValidator(),
    SwipeValidator(x="==1223 ~ 400"),
    SwipeLiftValidator(),
    SwipeValidator(y="==1657 ~ 400"),
    SwipeLiftValidator(),
    SwipeValidator(y="==1554 ~ 400"),
    SwipeLiftValidator(),
  ]
  fuzzy.unexpected = [
    FlingStopValidator("<10"),
  ]
  return fuzzy.Check(gestures)
