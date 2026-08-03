# Copyright 2024 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from fuzzy_check import FuzzyCheck
from validators import *

# In the original recording, we locked on to the wrong finger and missed most
# motion.

def Validate(raw, events, gestures):
  fuzzy = FuzzyCheck()
  fuzzy.expected = [
    ButtonDownValidator(1),
    MotionValidator(">= 400"),
    ButtonUpValidator(1),
  ]
  fuzzy.unexpected = [
    MotionValidator("<10"),
    MotionValidator("<10"),
    MotionValidator("<10"),
    FlingStopValidator("<10"),
  ]
  return fuzzy.Check(gestures)
