# Copyright (c) 2013 The Chromium OS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

from fuzzy_check import FuzzyCheck
from validators import *

# originally generated gestures:
# Scroll d=938 x=0 y=938 r=22.22
#   Fling d=896.5 x=0 y=896.5 r=1.137e-13

def Validate(raw, events, gestures):
  fuzzy = FuzzyCheck()
  fuzzy.expected = [
    ScrollValidator(">10"),
  ]
  fuzzy.unexpected = [
    FlingStopValidator("<10"),
    FlingValidator(),
  ]
  return fuzzy.Check(gestures)
