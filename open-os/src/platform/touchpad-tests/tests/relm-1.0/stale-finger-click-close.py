# Copyright 2017 The Chromium OS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

from fuzzy_check import FuzzyCheck
from validators import *

# A finger which moves close to a stale finger and then clicks should result in
# a left-click

def Validate(raw, events, gestures):
  fuzzy = FuzzyCheck()
  fuzzy.expected = [
    MotionValidator(">0"),
    ButtonDownValidator(1),
    ButtonUpValidator(1),
  ]
  fuzzy.unexpected = [
    FlingStopValidator("<10"),
    MotionValidator("<10"),
  ]
  return fuzzy.Check(gestures)
