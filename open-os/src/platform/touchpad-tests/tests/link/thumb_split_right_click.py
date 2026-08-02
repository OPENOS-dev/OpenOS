# Copyright (c) 2013 The Chromium OS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

from fuzzy_check import FuzzyCheck
from validators import *

# originally generated gestures:
# FlingStop
#   Motion d=1573 x=1500 y=385.8 r=5.107 s=4694
#   FlingStop
#   Motion d=54.39 x=53.17 y=6.912 r=0.4097 s=221.2
#   FlingStop
#   ButtonDown(4)
#   ButtonUp(4)
#   Motion d=2747 x=2504 y=1021 r=6.497 s=4732

def Validate(raw, events, gestures):
  fuzzy = FuzzyCheck()
  fuzzy.expected = [
    ButtonDownValidator(1),
    ButtonUpValidator(1),
  ]
  fuzzy.unexpected = [
    MotionValidator(merge=True),
    FlingStopValidator("<10"),
  ]
  return fuzzy.Check(gestures)
