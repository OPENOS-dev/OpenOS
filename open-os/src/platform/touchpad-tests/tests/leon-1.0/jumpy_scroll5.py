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
#   Motion d=2.34 x=0.4468 y=2.234 r=0.146 s=52.91
#   Scroll d=693.3 x=120 y=586 r=3.305 s=172.3
#   Fling d=0 x=0 y=0 r=0 s=0

def Validate(raw, events, gestures):
  fuzzy = FuzzyCheck()
  fuzzy.expected = [
    ScrollValidator("= 541.7 ~ 25.0", merge=True),
    FlingValidator()
  ]
  fuzzy.unexpected = [
    FlingStopValidator("<10"),
  ]
  return fuzzy.Check(gestures)
