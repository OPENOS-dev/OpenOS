# Copyright 2018 The Chromium OS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

from fuzzy_check import FuzzyCheck
from validators import *

# Holding button at bottom and touching touchpad causes big jumps if 0 pressure
# fingers are not filtered.

def Validate(raw, events, gestures):
  fuzzy = FuzzyCheck()
  fuzzy.expected = [
    ButtonDownValidator(1),
    ButtonUpValidator(1),
    AnythingButValidator(ButtonDownValidator(1)),
    ButtonDownValidator(1),
  ]
  fuzzy.unexpected = [
    FlingStopValidator("<10"),
    MotionValidator("< 10"),
    MotionValidator("< 10"),
  ]
  return fuzzy.Check(gestures)
