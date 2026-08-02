# Copyright 2022 The Chromium OS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from fuzzy_check import FuzzyCheck
from validators import *

# This is an example of a case where the haptic button generator would release
# a click at an unexpected time.  There should be only one set of ButtonDown and
# ButtonUp events here, not two.

def Validate(raw, events, gestures):
  fuzzy = FuzzyCheck()
  fuzzy.expected = [
    ButtonDownValidator(1),
    MotionValidator(">0"),
    ButtonUpValidator(1),
  ]
  fuzzy.unexpected = [
    FlingStopValidator("<10"),
  ]
  return fuzzy.Check(gestures)
