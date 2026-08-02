# Copyright 2017 The Chromium OS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

from fuzzy_check import FuzzyCheck
from validators import *

# One finger arriving near a stationary finger and then moving should result in
# a move gesture, not a scroll gesture

def Validate(raw, events, gestures):
  fuzzy = FuzzyCheck()
  fuzzy.expected = [
  ]
  fuzzy.unexpected = [
    MotionValidator(">= 750 ~ 100"),
    FlingStopValidator("<10"),
  ]
  return fuzzy.Check(gestures)
