# Copyright 2018 The Chromium OS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

from fuzzy_check import FuzzyCheck
from validators import *

# Note: the absinfo for the orientation axis in the log file for this test was
# originally `# absinfo: 52 -90 90 0 0 0`, due to it being recorded with
# pre-production firmware. It has been changed to `# absinfo: 52 0 1 0 0 0` so
# that the platform files can be found.

def Validate(raw, events, gestures):
  fuzzy = FuzzyCheck()
  fuzzy.expected = [
    ScrollValidator(">= 750"),
    FlingValidator(">= 0"),
    SwipeValidator("== 81.98 ~ 8", merge=True),
    SwipeLiftValidator(),
  ]
  fuzzy.unexpected = [
    FlingStopValidator("<10"),
  ]
  return fuzzy.Check(gestures)
