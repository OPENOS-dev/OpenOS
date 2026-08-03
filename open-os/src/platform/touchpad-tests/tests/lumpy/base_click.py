# Copyright (c) 2012 The Chromium OS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

from fuzzy_check import FuzzyCheck
from validators import *

# originally generated gestures:
# FlingStop
#   FlingStop
#   ButtonDown(1)
#   ButtonUp(1)
#   ButtonDown(4)
#   ButtonUp(4)
#   ButtonDown(1)
#   FlingStop
#   ButtonUp(1)
#   ButtonDown(4)
#   FlingStop
#   ButtonUp(4)

def Validate(raw, events, gestures):
  fuzzy = FuzzyCheck()
  fuzzy.expected = [
    ButtonDownValidator(1),
    ButtonUpValidator(1),
    ButtonDownValidator(4),
    ButtonUpValidator(4),
    ButtonDownValidator(1),
    ButtonUpValidator(1),
    ButtonDownValidator(4),
    ButtonUpValidator(4)
  ]
  fuzzy.unexpected = [
    MotionValidator("<10"),
    FlingStopValidator("<10"),
  ]
  return fuzzy.Check(gestures)
