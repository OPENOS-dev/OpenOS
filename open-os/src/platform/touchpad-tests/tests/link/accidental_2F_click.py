# Copyright (c) 2012 The Chromium OS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

from fuzzy_check import FuzzyCheck
from validators import *

# originally generated gestures:
# Motion d=39 x=36 y=10 r=0.24
# FlingStop
# ButtonDown(4)
# FlingStop
# ButtonUp(4)
# Motion d=33 x=0 y=33 r=0.91
# FlingStop

def Validate(raw, events, gestures):
  fuzzy = FuzzyCheck()
  fuzzy.expected = [
    ButtonDownValidator(1),
    ButtonUpValidator(1)
  ]
  fuzzy.unexpected = [
    FlingStopValidator("<10"),
    MotionValidator("<200", merge=True)
  ]

  return fuzzy.Check(gestures)
