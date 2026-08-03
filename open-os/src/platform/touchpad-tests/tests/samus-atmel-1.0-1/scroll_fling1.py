# Copyright 2015 The Chromium OS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

from fuzzy_check import FuzzyCheck
from validators import *

# originally generated gestures:
# FlingStop
#   Scroll d=411.4 x=83.92 y=374.5 r=1.379 s=752.5
#   Scroll d=1.745 x=1.745 y=0 r=1.789e-05 s=17.73
#   ButtonDown(1)
#   ButtonUp(1)
#   FlingStop

def Validate(raw, events, gestures):
  fuzzy = FuzzyCheck()
  fuzzy.expected = [
    ScrollValidator("== 344.3 ~ 41.7", merge=True),
    FlingValidator("<= 10"),
  ]
  fuzzy.unexpected = [
    FlingStopValidator("<10"),
  ]
  return fuzzy.Check(gestures)
