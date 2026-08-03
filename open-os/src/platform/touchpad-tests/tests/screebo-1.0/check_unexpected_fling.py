# Copyright 2025 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from fuzzy_check import FuzzyCheck
from validators import *

# This test verifies that a pause after a scroll gesture results
# in a zero-velocity fling, rather than a non-zero velocity fling.

def Validate(raw, events, gestures):
  fuzzy = FuzzyCheck()
  fuzzy.expected = [
    FlingStopValidator(),
    ScrollValidator(">= 0"),
    FlingValidator("== 0"),
  ]
  return fuzzy.Check(gestures)
