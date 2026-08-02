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
    This test was artificially created. Both fingers touch down at exactly the
    same time, which is not usually happening for thumb and finger.
    So we treat them as both fingers and right click.
  """
  fuzzy = FuzzyCheck()
  fuzzy.expected = [
    ButtonDownValidator(4),
    ButtonUpValidator(4),
  ]
  fuzzy.unexpected = [
    MotionValidator("<10"),
    FlingStopValidator("<10"),
  ]
  return fuzzy.Check(gestures)
