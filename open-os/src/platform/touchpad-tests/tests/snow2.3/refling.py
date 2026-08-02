# Copyright (c) 2012 The Chromium OS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

from fuzzy_check import FuzzyCheck
from validators import *

# originally generated gestures:
# Scroll d=367 x=7 y=365 r=0.35
# Fling d=866 x=0 y=866 r=0.00
# Scroll d=288 x=0 y=288 r=1.14
# Fling d=2099 x=0 y=2099 r=0.00
# Scroll d=99 x=3 y=96 r=0.35
# Scroll d=79 x=3 y=76 r=0.27
# Scroll d=98 x=2 y=96 r=0.30
# Fling d=612 x=0 y=612 r=0.00
# Scroll d=289 x=0 y=289 r=1.55
# Fling d=3477 x=0 y=3477 r=0.00


def Validate(raw, events, gestures):
  fuzzy = FuzzyCheck()
  fuzzy.expected = [
    FlingStopValidator("1"),
    ScrollValidator("== 268.3 ~ 208.3"),
    FlingValidator("> 0"),
    FlingStopValidator("1"),
    ScrollValidator("== 349.2 ~ 250.0"),
    FlingValidator("> 0"),
    FlingStopValidator("1"),
    ScrollValidator("< 833.3", merge=True),
    FlingValidator("> 0"),
    FlingStopValidator("1"),
    ScrollValidator("== 345.0 ~ 250.0"),
    FlingValidator("> 0"),
  ]
  fuzzy.unexpected = [
  ]
  return fuzzy.Check(gestures)
