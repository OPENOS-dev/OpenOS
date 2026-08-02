# Copyright 2024 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from fuzzy_check import FuzzyCheck
from validators import *

# Slow scrolls should only generate zero-length flings

def Validate(raw, events, gestures):
  fuzzy = FuzzyCheck()
  fuzzy.expected = [
  ]
  fuzzy.unexpected = [
    AnythingButValidator(FlingValidator(">0")),
  ]
  return fuzzy.Check(gestures)
