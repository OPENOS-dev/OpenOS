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
#   Motion d=21 x=2 y=21 r=0.36
#   FlingStop
#   ButtonDown(1)
#   ButtonUp(1)
#   Motion d=2 x=0 y=2 r=0.00

# The important thing is that the tap happens, since it is at the very top
# of the pad, where previously palm suppression filtered it out.

def Validate(raw, events, gestures):
  fuzzy = FuzzyCheck()
  fuzzy.expected = [
    ButtonDownValidator(1),
    ButtonUpValidator(1),
  ]
  fuzzy.unexpected = [
    MotionValidator("<30 ~ 30"),
    FlingStopValidator("<10"),
  ]
  return fuzzy.Check(gestures)
