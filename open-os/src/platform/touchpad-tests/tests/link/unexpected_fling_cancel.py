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
#   FlingStop
#   Scroll d=247 x=247 y=0 r=6.034 s=1912
#   Fling d=1165 x=1165 y=0 r=0 s=1.365e+05
#   FlingStop

def Validate(raw, events, gestures):
  fuzzy = FuzzyCheck()
  fuzzy.expected = [
    FlingStopValidator(),
    ScrollValidator(">= 100"),
    FlingValidator(">= 100", merge=True),
  ]
  fuzzy.unexpected = [
  ]
  return fuzzy.Check(gestures)
