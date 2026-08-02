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
# FlingStop
# Motion d=14 x=1 y=13 r=0.46
# Scroll d=419 x=13 y=409 r=1.21
# Fling d=0 x=0 y=0 r=0.00


def Validate(raw, events, gestures):
  fuzzy = FuzzyCheck()
  fuzzy.expected = [
    ScrollValidator("== 349.2 ~ 125.0"),
    FlingValidator("== 0"),
  ]
  fuzzy.unexpected = [
    FlingStopValidator("<5"),
    MotionValidator("<30", merge=True)
  ]
  return fuzzy.Check(gestures)
