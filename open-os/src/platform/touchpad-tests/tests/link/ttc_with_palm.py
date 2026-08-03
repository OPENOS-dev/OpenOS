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
    The log contains some history of the palm with movement/scrolls.
    At the end there are two taps performed and we want to make sure
    they happen.
  """
  fuzzy = FuzzyCheck()
  fuzzy.expected = [
    AnythingButValidator(ButtonDownValidator(1)),
    ButtonDownValidator(1),
    ButtonUpValidator(1),
    ButtonDownValidator(1),
    ButtonUpValidator(1),
  ]
  fuzzy.unexpected = [
    MotionValidator("0 ~ 2"),
    FlingStopValidator("<10"),
  ]
  return fuzzy.Check(gestures)
