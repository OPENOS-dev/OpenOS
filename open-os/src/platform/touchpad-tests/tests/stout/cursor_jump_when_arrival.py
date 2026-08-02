# Copyright (c) 2013 The Chromium OS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

from fuzzy_check import FuzzyCheck
from validators import *

# originally generated gestures:
#   Motion d=135.6 x=33.52 y=131.2 r=32.91
#   FlingStop

def Validate(raw, events, gestures):
  # A cursor jump is unexpected when a finger arrives. The fix is created by the
  # fact that the jump comes with a high pressure change. However, there will be
  # a little shift caused by instablility of finger arrival.

  fuzzy = FuzzyCheck()
  fuzzy.expected = [
    MotionValidator("==0 ~ 10", merge=True),
  ]
  fuzzy.unexpected = [
    FlingStopValidator("<10"),
  ]
  return fuzzy.Check(gestures)
