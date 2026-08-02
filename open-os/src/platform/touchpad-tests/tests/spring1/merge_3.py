# Copyright (c) 2014 The Chromium OS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

from fuzzy_check import FuzzyCheck
from validators import *

# originally generated gestures:
# FlingStop
#   Motion d=550.5 x=473 y=133.4 r=101.5 s=7975
#   Scroll d=1392 x=21 y=1383 r=80.17 s=1.121e+04
#   Fling d=2.785e+04 x=0 y=2.785e+04 r=3.638e-12 s=2.524e+06
#   Fling d=161.7 x=114.4 y=114.4 r=0 s=1.218e+04

def Validate(raw, events, gestures):
  fuzzy = FuzzyCheck()
  fuzzy.expected = [
  ]
  fuzzy.unexpected = [
    FlingStopValidator("<10"),
    ScrollValidator(merge=True),
    FlingValidator(),
  ]
  return fuzzy.Check(gestures)
