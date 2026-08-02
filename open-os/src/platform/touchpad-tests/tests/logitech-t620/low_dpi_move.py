# Copyright (c) 2014 The Chromium OS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

from fuzzy_check import FuzzyCheck
from validators import *

# originally generated gestures:
# Motion d=0.6991 x=0.6991 y=0 r=1.11e-16 s=0.566

def Validate(raw, events, gestures):
  fuzzy = FuzzyCheck()
  fuzzy.expected = [
    MotionValidator("== 0.833333 ~ 0.001"),
  ]
  fuzzy.unexpected = [
  ]
  return fuzzy.Check(gestures)
