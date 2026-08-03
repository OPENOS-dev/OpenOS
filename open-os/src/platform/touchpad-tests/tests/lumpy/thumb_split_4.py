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
#   Motion d=0 x=0 y=0 r=0.05
#   FlingStop
#   ButtonDown(4)
#   ButtonUp(4)

def Validate(raw, events, gestures):
  fuzzy = FuzzyCheck()
  # We don't care about motion in this test. We just want a single left click.
  fuzzy.expected = [
    ButtonDownValidator(1),
    ButtonUpValidator(1),
  ]
  fuzzy.unexpected = [
    MotionValidator(merge=True),
    FlingStopValidator("<10"),
  ]
  return fuzzy.Check(gestures)
