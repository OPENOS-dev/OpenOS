# Copyright (c) 2012 The Chromium OS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

from fuzzy_check import FuzzyCheck
from validators import *

# originally generated gestures:
# Motion d=226 x=205 y=93 r=7.94
#   FlingStop
#   Scroll d=75 x=6 y=70 r=1.17
#   Scroll d=0 x=0 y=0 r=0.00
#   Scroll d=50 x=0 y=50 r=1.54
#   Scroll d=10 x=0 y=10 r=0.41
#   Fling d=36 x=0 y=36 r=0.00

def Validate(raw, events, gestures):
  fuzzy = FuzzyCheck()
  fuzzy.expected = [
    # MotionValidator("== 100 ~ 50"),
    # FlingStopValidator(),
    # ButtonDownValidator(1),
    # ButtonUpValidator(1),
    # ScrollValidator(">= 100"),
    # FlingValidator(">= 100"),
    # AnythingButValidator(ButtonDownValidator(1)),
  ]
  fuzzy.unexpected = [
    # MotionValidator("<10"),
    FlingStopValidator("<10"),
    ScrollValidator(merge=True),
    FlingValidator(),
  ]
  return fuzzy.Check(gestures)
