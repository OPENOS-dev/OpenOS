# Copyright (c) 2012 The Chromium OS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

from fuzzy_check import FuzzyCheck
from validators import *

# Originally generated gestures:
# ButtonDown(1)
# ButtonUp(1)
# Motion d=223 x=196 y=80 r=1.35
# FlingStop
# ButtonDown(1)
# ButtonUp(1)
# FlingStop
# FlingStop
# Motion d=84 x=45 y=71 r=4.48

def Validate(raw, events, gestures):
  fuzzy = FuzzyCheck()
  fuzzy.expected = [
    MotionValidator("<300"),
    ButtonDownValidator(1),
    ButtonUpValidator(1),
    MotionValidator("<100")
  ]
  fuzzy.unexpected = [
    FlingStopValidator("== 3 ~ 1")
  ]

  return fuzzy.Check(gestures)
