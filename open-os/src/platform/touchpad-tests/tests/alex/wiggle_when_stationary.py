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
# Motion d=6.192 x=4.057 y=3.976 r=0.01225

# Wiggling from arrival to liftoff @168088.748093 - 168090.727988
# After enabling Stationary Wiggle Filter, there is only small motion from
# the instabilty of finger arrival @168088.748093 - 168088.796898

def Validate(raw, events, gestures):
  fuzzy = FuzzyCheck()
  fuzzy.expected = [
    MotionValidator("==0 ~ 10")
  ]
  fuzzy.unexpected = [
    FlingStopValidator("<10"),
  ]
  return fuzzy.Check(gestures)
