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
#   Scroll d=255 x=0 y=255 r=3.618 s=518.2
#   Fling d=0 x=0 y=0 r=0 s=0
#   Motion d=0.3128 x=0.2129 y=0.09984 r=0.009027 s=0.6982
#   Motion d=0.07389 x=0.07389 y=0 r=0 s=0.4271

def Validate(raw, events, gestures):
  fuzzy = FuzzyCheck()
  fuzzy.expected = [
    ScrollValidator(">= 100"),
  ]
  fuzzy.unexpected = [
    MotionValidator("<10", merge=True),
    FlingStopValidator("<10"),
  ]
  return fuzzy.Check(gestures)
