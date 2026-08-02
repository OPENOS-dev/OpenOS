# Copyright (c) 2012 The Chromium OS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

from fuzzy_check import FuzzyCheck
from validators import *


def Validate(raw, events, gestures):
  """
    Issues with finger merging appear when multiple fingers touch down at
    the same time. These issues are usually resolved after 2, 3 hardware
    states. This test case shows the problem with a 3 finger click.
  """
  fuzzy = FuzzyCheck()
  fuzzy.expected = [
    ButtonDownValidator(2),
    ButtonUpValidator(2),
  ]
  fuzzy.unexpected = [
    MotionValidator("=0"),
    FlingStopValidator("<10"),
  ]
  return fuzzy.Check(gestures)
