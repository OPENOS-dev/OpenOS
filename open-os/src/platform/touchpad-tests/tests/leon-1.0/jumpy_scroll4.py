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
#   Scroll d=657.1 x=89 y=588 r=2.889 s=293.4
#   Scroll d=48 x=4 y=44 r=4.789 s=175.3
#   Scroll d=22 x=2 y=20 r=0.8405 s=115.4
#   Scroll d=189.8 x=43 y=148 r=2.886 s=131
#   Scroll d=51 x=48 y=3 r=4.76 s=233.6
#   Scroll d=394.9 x=144 y=273 r=10.68 s=390.7
#   Scroll d=59.41 x=41 y=43 r=7.105e-15 s=4347
#   Scroll d=55.15 x=39 y=39 r=1.044 s=808.2
#   Scroll d=45.3 x=25 y=34 r=4.865 s=845.5
#   Fling d=0 x=0 y=0 r=0 s=0

def Validate(raw, events, gestures):
  fuzzy = FuzzyCheck()
  fuzzy.expected = [
    ScrollValidator("= 572.5 ~ 25.0", merge=True),
    FlingValidator()
  ]
  fuzzy.unexpected = [
    FlingStopValidator("<10"),
  ]
  return fuzzy.Check(gestures)
