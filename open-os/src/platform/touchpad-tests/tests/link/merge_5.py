# Copyright (c) 2012 The Chromium OS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

from fuzzy_check import FuzzyCheck
from validators import *

# originally generated gestures:
# Motion d=369 x=367 y=16 r=18.99
#   FlingStop
#   Scroll d=57 x=44 y=13 r=4.03
#   Scroll d=1 x=0 y=1 r=0.00
#   FlingStop

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
