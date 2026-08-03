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
#   Motion d=851 x=716 y=316 r=2.04
#   Motion d=11 x=8 y=3 r=0.00
#   Motion d=813 x=727 y=202 r=2.92
#   FlingStop

def Validate(raw, events, gestures):
  fuzzy = FuzzyCheck()
  fuzzy.expected = [
    ButtonDownValidator(1),
    ButtonUpValidator(1),
  ]
  fuzzy.unexpected = [
    # Only asserting the click in this test. We have lots of other tests that
    # assert things about motion
    MotionValidator(">0", merge=True),
    FlingStopValidator("<10"),
  ]
  return fuzzy.Check(gestures)
