# Copyright 2017 The Chromium OS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

from fuzzy_check import FuzzyCheck
from validators import *

# Two fingers moving with a third stationary finger should result in a scroll
# gesture with a fling gesture after

def Validate(raw, events, gestures):
  fuzzy = FuzzyCheck()
  fuzzy.expected = [
    ScrollValidator(),
    FlingValidator(),
    ScrollValidator(),
    FlingValidator(),
    ScrollValidator(),
    FlingValidator(),
    ScrollValidator(),
    FlingValidator(),
  ]
  fuzzy.unexpected = [
    FlingStopValidator("<10"),
  ]
  return fuzzy.Check(gestures)
