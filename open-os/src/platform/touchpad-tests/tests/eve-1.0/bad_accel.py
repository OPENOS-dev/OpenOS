# Copyright 2017 The Chromium OS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

from fuzzy_check import FuzzyCheck
from validators import *

def Validate(raw, events, gestures):
  for e in events:
    if e.type == "Motion":
      biggest_move = max(e.segments)
      break
  good = 7.5
  bad = 12.0
  score = (bad - biggest_move) / (bad - good)
  score = min(1.0, max(0.0, score))
  report = ("Max motion in a single update was {0}.\n"
            "Motion below {1} is scored 1.0, "
            "and above {2} is scored 0.0").format(biggest_move, good, bad)
  return score, report
