# Copyright (c) 2012 The Chromium OS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

from fuzzy_check import FuzzyCheck
from validators import *

# originally generated gestures:
# ButtonDown(1)
#   FlingStop
#   ButtonUp(1)
#   ButtonDown(1)
#   FlingStop
#   ButtonUp(1)
#   ButtonDown(1)
#   FlingStop
#   ButtonUp(1)
#   ButtonDown(1)
#   FlingStop
#   ButtonUp(1)
#   ButtonDown(1)
#   FlingStop
#   ButtonUp(1)
#   ButtonDown(1)
#   FlingStop
#   ButtonUp(1)
#   ButtonDown(1)
#   FlingStop
#   ButtonUp(1)
#   ButtonDown(1)
#   FlingStop
#   ButtonUp(1)
#   ButtonDown(1)
#   FlingStop
#   ButtonUp(1)
#   ButtonDown(1)
#   FlingStop
#   ButtonUp(1)

def Validate(raw, events, gestures):
  fuzzy = FuzzyCheck()
  fuzzy.expected = [
    ButtonDownValidator(1),
    ButtonUpValidator(1),
    ButtonDownValidator(1),
    ButtonUpValidator(1),
    ButtonDownValidator(1),
    ButtonUpValidator(1),
    ButtonDownValidator(1),
    ButtonUpValidator(1),
    ButtonDownValidator(1),
    ButtonUpValidator(1),
    ButtonDownValidator(1),
    ButtonUpValidator(1),
    ButtonDownValidator(1),
    ButtonUpValidator(1),
    ButtonDownValidator(1),
    ButtonUpValidator(1),
    ButtonDownValidator(1),
    ButtonUpValidator(1),
    ButtonDownValidator(1),
    ButtonUpValidator(1),
  ]
  fuzzy.unexpected = [
    FlingStopValidator("<20"),
    MotionValidator("== 0 ~ 50"),
  ]
  return fuzzy.Check(gestures)
