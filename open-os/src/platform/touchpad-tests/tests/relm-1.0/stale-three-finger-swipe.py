# Copyright 2017 The Chromium OS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

from fuzzy_check import FuzzyCheck
from validators import *

# Three fingers moving with a fourth stationary finger should result in a three
# finger swipe gesture only

def Validate(raw, events, gestures):
  fuzzy = FuzzyCheck()
  fuzzy.expected = [
    SwipeValidator(),
    SwipeValidator(),
    SwipeValidator(),
    SwipeValidator(),
  ]
  fuzzy.unexpected = [
    FlingStopValidator("<10"),
    SwipeLiftValidator("== 4"),
  ]
  return fuzzy.Check(gestures)
