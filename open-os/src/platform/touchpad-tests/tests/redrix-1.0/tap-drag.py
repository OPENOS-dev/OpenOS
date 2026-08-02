# Copyright 2023 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from fuzzy_check import FuzzyCheck
from validators import *

# Make sure that a tap-to-click is generated after a long pause, not a tap-drag.

def Validate(raw, events, gestures):
  fuzzy = FuzzyCheck()
  fuzzy.expected = [
    AnythingButValidator(ButtonDownValidator(1)),
    ButtonDownValidator(1),
    ButtonUpValidator(1),
    MotionValidator(),
  ]
  fuzzy.unexpected = [
    FlingStopValidator("<10"),
  ]
  return fuzzy.Check(gestures)
