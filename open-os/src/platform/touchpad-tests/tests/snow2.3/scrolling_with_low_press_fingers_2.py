# Copyright (c) 2012 The Chromium OS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

from fuzzy_check import FuzzyCheck
from validators import *

# originally generated gestures:
# Scroll d=1234 x=0 y=1234 r=36.59
#   Fling d=11770 x=0 y=11770 r=0.00

def Validate(raw, events, gestures):
  fuzzy = FuzzyCheck()
  fuzzy.expected = [
    FlingStopValidator(),
    ScrollValidator(">= 1000.0 ~ 416.7"),
    FlingValidator(),
  ]
  fuzzy.unexpected = [
  ]
  return fuzzy.Check(gestures)
