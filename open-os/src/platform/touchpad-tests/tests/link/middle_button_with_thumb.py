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
    Classified thumbs used to induce problems with button detections,
    check if the middle click works.
  """
  fuzzy = FuzzyCheck()
  fuzzy.expected = [
    ButtonDownValidator(2),
    ButtonUpValidator(2),
  ]
  fuzzy.unexpected = [
    MotionValidator("<10", merge=True),
    FlingStopValidator("<10"),
  ]
  return fuzzy.Check(gestures)
