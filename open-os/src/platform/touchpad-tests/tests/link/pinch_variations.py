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
    Pinch is currently not supported and work in progress.
  """
  fuzzy = FuzzyCheck()
  fuzzy.expected = [
    PinchValidator("> 100 ~ 90"),
    PinchValidator("> 100 ~ 90"),
    PinchValidator("> 100 ~ 90"),
    PinchValidator("> 100 ~ 90"),
    PinchValidator("> 100 ~ 90"),
    PinchValidator("> 100 ~ 90"),
    PinchValidator("> 100 ~ 90"),
    PinchValidator("> 100 ~ 90"),
    PinchValidator("> 100 ~ 90"),
  ]
  fuzzy.unexpected = [
    MotionValidator("<1000", merge=True),
    ScrollValidator("<1000", merge=True),
    FlingStopValidator("< 20")
  ]
  return fuzzy.Check(gestures)

Validate.disabled = True
