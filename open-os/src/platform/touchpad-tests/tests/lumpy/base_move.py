# Copyright (c) 2012 The Chromium OS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

from fuzzy_check import FuzzyCheck
from validators import *

# originally generated gestures:
# Motion d=3216 x=2320 y=1640 r=2.95
#   FlingStop

def Validate(raw, events, gestures):
  fuzzy = FuzzyCheck()
  fuzzy.expected = [
    MotionValidator(">3200~1000")
  ]
  fuzzy.unexpected = [
    MotionValidator("<10"),
    FlingStopValidator("<10")
  ]
  return fuzzy.Check(gestures)
