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
# Scroll d=385 x=385 y=0 r=0.81
# Fling d=75 x=75 y=0 r=0.00

def Validate(raw, events, gestures):
  fuzzy = FuzzyCheck()
  fuzzy.expected = [
    ScrollValidator("== 385 ~ 200"),
    FlingValidator("== 75 ~ 100"),
  ]
  fuzzy.unexpected = [
    FlingStopValidator("<10"),
  ]
  return fuzzy.Check(gestures)
