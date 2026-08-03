# Copyright (c) 2012 The Chromium OS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

from fuzzy_check import FuzzyCheck
from validators import *

# originally generated gestures:
# Scroll d=1326 x=2 y=1324 r=42.13
#   Fling d=13654 x=0 y=13654 r=0.00

def Validate(raw, events, gestures):
  fuzzy = FuzzyCheck()
  fuzzy.expected = [
    FlingStopValidator(),
    ScrollValidator(">= 1083.3 ~ 416.7"),
    FlingValidator(),
  ]
  fuzzy.unexpected = [
  ]
  return fuzzy.Check(gestures)
