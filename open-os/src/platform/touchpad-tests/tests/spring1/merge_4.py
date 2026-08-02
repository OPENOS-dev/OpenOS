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
#   Motion d=311.6 x=309.4 y=36.3 r=15.87 s=3402
#   FlingStop
#   Scroll d=8.414 x=7 y=2 r=0.1014 s=56.91

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
