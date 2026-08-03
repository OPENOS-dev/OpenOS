# Copyright (c) 2013 The Chromium OS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

from fuzzy_check import FuzzyCheck
import mtlib.gesture_log
from validators import *

def Validate(raw, events, gestures):
  fuzzy = FuzzyCheck()
  # Most important thing about this test: make sure that we don't get any motion
  # after the original finger departs
  for gs in gestures:
    if isinstance(gs, mtlib.gesture_log.MotionGesture):
      if gs.end > 699.14048:
        return False, ('Error: Move end time is too late: %f' % gs.end)
  fuzzy.expected = [
    MotionValidator(merge=True),
    ButtonDownValidator(1),
    MotionValidator(merge=True),
    ButtonUpValidator(1),
  ]
  fuzzy.unexpected = [
    FlingStopValidator("<10"),
  ]
  return fuzzy.Check(gestures)
