# Copyright 2022 The Chromium OS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from fuzzy_check import FuzzyCheck
from validators import *

# This test was captured while reproducing a bug where the user could tap once
# without triggering tap-to-click, and then seconds later when they moved the
# cursor, a tap-drag was improperly triggered.
#
# We expect a tap-to-click to trigger, and then no tap-drag.

def Validate(raw, events, gestures):
  fuzzy = FuzzyCheck()
  fuzzy.expected = [
    ButtonDownValidator(1),
    ButtonUpValidator(1),
    MotionValidator(),
  ]
  fuzzy.unexpected = [
    FlingStopValidator("<10"),
  ]
  return fuzzy.Check(gestures)
