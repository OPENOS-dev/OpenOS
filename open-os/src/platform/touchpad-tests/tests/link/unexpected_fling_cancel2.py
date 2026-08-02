# Copyright (c) 2014 The Chromium OS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

from fuzzy_check import FuzzyCheck
from validators import *

# originally generated gestures:
# Scroll d=407 x=407 y=0 r=22.33 s=3948
#   Fling d=3969 x=3969 y=0 r=4.547e-13 s=4.936e+05
#   FlingStop

def Validate(raw, events, gestures):
  fuzzy = FuzzyCheck()
  fuzzy.expected = [
    FlingStopValidator(),
    ScrollValidator(">= 100"),
    FlingValidator(">= 100"),
  ]
  fuzzy.unexpected = [
  ]
  return fuzzy.Check(gestures)
