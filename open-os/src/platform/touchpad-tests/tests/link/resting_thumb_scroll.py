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
    Test if even with a resting (and moving) thumb we still get
    good scrolling and fling behavior
  """
  fuzzy = FuzzyCheck()
  fuzzy.expected = [
    ScrollValidator("> 200 ~ 100"),
    # The important thing about this test is that a good fling occurs:
    FlingValidator("> 2000 ~ 1000"),
  ]
  fuzzy.unexpected = [
    FlingStopValidator("<10"),
  ]
  return fuzzy.Check(gestures)
