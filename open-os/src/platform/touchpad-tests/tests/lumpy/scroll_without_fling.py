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
    Make sure there is no *real* fling.  Every time a scroll ends there
    should be a fling event, but in this case it's velocity must be 0
  """
  fuzzy = FuzzyCheck()
  fuzzy.expected = [
    ScrollValidator("> 200 ~ 100"),
    FlingValidator("0"),
  ]
  fuzzy.unexpected = [
    FlingStopValidator("<10"),
  ]
  return fuzzy.Check(gestures)
