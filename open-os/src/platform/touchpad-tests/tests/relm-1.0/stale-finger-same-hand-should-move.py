# Copyright 2017 The Chromium OS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

from fuzzy_check import FuzzyCheck
from validators import *

# Single-finger scrolling is disabled. A non-moving finger, even on the same
# hand as a moving finger, should be ignored.

def Validate(raw, events, gestures):
  fuzzy = FuzzyCheck()
  fuzzy.expected = [
  ]
  fuzzy.unexpected = [
    MotionValidator(">= 0"),
    MotionValidator(">= 800 ~ 200"),
    FlingStopValidator("<10"),
  ]
  return fuzzy.Check(gestures)
