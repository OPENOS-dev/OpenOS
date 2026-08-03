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
#   Scroll d=65 x=9 y=58 r=0.77
#   Fling d=0 x=0 y=0 r=0.00
#   FlingStop
#   Motion d=12 x=1 y=10 r=0.20
#   Motion d=50 x=8 y=45 r=0.27
#
# correct one should be without motion gestures:
#FlingStop
#   Scroll d=65 x=9 y=58 r=0.77
#   Fling d=0 x=0 y=0 r=0.00
#   FlingStop
#   Scroll d=277 x=16 y=264 r=0.83
#   Fling d=0 x=0 y=0 r=0.00

def Validate(raw, events, gestures):
  fuzzy = FuzzyCheck()
  fuzzy.expected = [
    ScrollValidator(">= 50.0"),
    FlingValidator("<= 8.3"),
    ScrollValidator(">= 208.3"),
    FlingValidator("<= 8.3"),
  ]
  fuzzy.unexpected = [
    FlingStopValidator("<10")
  ]
  return fuzzy.Check(gestures)
