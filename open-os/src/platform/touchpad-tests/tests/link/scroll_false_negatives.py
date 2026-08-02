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
    A scrolling gesture with a resting thumb. Tricky part here is
    that the thumb has only a slightly higher pressure than
    the other contacts. Make sure scrolling works anyway.
  """
  fuzzy = FuzzyCheck()
  fuzzy.expected = [
    ScrollValidator(">= 50"),
    FlingValidator(">=0")
  ]
  fuzzy.unexpected = [
    MotionValidator("<10"),
    FlingStopValidator("<10"),
  ]
  return fuzzy.Check(gestures)
