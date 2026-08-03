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
#   Scroll d=320.1 x=62 y=272 r=1.052 s=113
#   Scroll d=46.41 x=20 y=27 r=1.065 s=97.41
#   Scroll d=23 x=0 y=23 r=0.5628 s=155.2
#   Scroll d=3 x=0 y=3 r=0.2987 s=42.09
#   Scroll d=3 x=3 y=0 r=4.441e-16 s=216.1
#   Scroll d=95.83 x=56 y=41 r=1.868 s=160
#   Scroll d=15 x=9 y=6 r=0.868 s=138.5
#   Scroll d=186 x=34 y=157 r=1.842 s=176.8
#   Fling d=193.8 x=193.8 y=0 r=2.842e-14 s=2.008e+04

def Validate(raw, events, gestures):
  fuzzy = FuzzyCheck()
  fuzzy.expected = [
    ScrollValidator("< 629.2~41.7", merge=True),
  ]
  fuzzy.unexpected = [
    FlingStopValidator("<10"),
    FlingValidator(),
  ]
  return fuzzy.Check(gestures)
