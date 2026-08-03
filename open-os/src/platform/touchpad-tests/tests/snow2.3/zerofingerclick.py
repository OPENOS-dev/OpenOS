# Copyright (c) 2012 The Chromium OS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

from fuzzy_check import FuzzyCheck
from validators import *

# originally generated gestures:
# ButtonDown(1)
# ButtonUp(1)
# FlingStop
# Motion d=11 x=11 y=0 r=0.16
# ButtonDown(1)
# ButtonUp(1)
# FlingStop

def Validate(raw, events, gestures):
  fuzzy = FuzzyCheck()
  fuzzy.expected = [
    MotionValidator("<20"),
    ButtonDownValidator(1),
    ButtonUpValidator(1)
  ]
  fuzzy.unexpected = [
    FlingStopValidator("== 2 ~ 1")
  ]

  return fuzzy.Check(gestures)
