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
    The beginning of this log shows small one-finger movements,
    so movement at the beginning is considered ok, while movement everywhere
    else should not happen.
  """
  fuzzy = FuzzyCheck()
  fuzzy.expected = [
    MotionValidator("< 20 ~ 10", merge=True),
    ButtonDownValidator(4),
    ButtonUpValidator(4),
  ]
  fuzzy.unexpected = [
    MotionValidator("== 0 ~ 2", merge=True),
    FlingStopValidator("<10"),
  ]
  return fuzzy.Check(gestures)
