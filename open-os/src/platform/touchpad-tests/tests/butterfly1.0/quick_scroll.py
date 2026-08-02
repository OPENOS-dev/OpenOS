# Copyright (c) 2012 The Chromium OS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

from fuzzy_check import FuzzyCheck
from validators import *

# originally generated gestures:
# Scroll d=772 x=0 y=772 r=0.00
#   Fling d=89279 x=0 y=89279 r=0.00

def Validate(raw, events, gestures):
  fuzzy = FuzzyCheck()
  fuzzy.expected = [
    FlingValidator("== 74399.2 ~ 8333.3"),
  ]
  fuzzy.unexpected = [
    ScrollValidator(),  # optional
    FlingStopValidator("<10"),
  ]
  return fuzzy.Check(gestures)
