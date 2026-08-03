# Copyright 2017 The Chromium OS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

from fuzzy_check import FuzzyCheck
from validators import *


def Validate(raw, events, gestures):
  fuzzy = FuzzyCheck()
  fuzzy.expected = [
    MotionValidator("> 400"),
    ButtonDownValidator(1),
    ButtonUpValidator(1),
    AnythingButValidator(ButtonDownValidator(1)),
    ButtonDownValidator(1),
    ButtonUpValidator(1),
    ButtonDownValidator(1),
    ButtonUpValidator(1),
    MotionValidator("> 550"),
    MotionValidator("> 850"),
    MotionValidator("> 50"),
    ButtonDownValidator(1),
    ButtonUpValidator(1),
    MotionValidator("> 400"),
  ]
  fuzzy.unexpected = [
    FlingStopValidator("<70"),
  ]
  return fuzzy.Check(gestures)
