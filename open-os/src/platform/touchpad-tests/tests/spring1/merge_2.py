# Copyright (c) 2014 The Chromium OS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

from fuzzy_check import FuzzyCheck
from validators import *

# originally generated gestures:
# Motion d=2188 x=2003 y=607.2 r=83.46 s=2.39e+04
#   FlingStop
#   Scroll d=153 x=0 y=153 r=6.272 s=1.178e+04
#   Fling d=1.968e+04 x=0 y=1.968e+04 r=3.638e-12 s=1.485e+06

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
