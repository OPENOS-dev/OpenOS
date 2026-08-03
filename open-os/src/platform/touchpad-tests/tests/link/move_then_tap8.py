# Copyright (c) 2012 The Chromium OS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

from fuzzy_check import FuzzyCheck
from validators import *

# originally generated gestures:
# Motion d=186 x=76 y=162 r=2.30
#   FlingStop
#   Motion d=0 x=0 y=0 r=0.00
#   ButtonDown(1)
#   ButtonUp(1)
#   FlingStop

def Validate(raw, events, gestures):
  fuzzy = FuzzyCheck()
  fuzzy.expected = [
    # The important thing is that the tap occurs. We don't test the motion here.
    ButtonDownValidator(1),
    ButtonUpValidator(1),
  ]
  fuzzy.unexpected = [
    MotionValidator(merge=True),
    FlingStopValidator("<10"),
  ]
  return fuzzy.Check(gestures)
