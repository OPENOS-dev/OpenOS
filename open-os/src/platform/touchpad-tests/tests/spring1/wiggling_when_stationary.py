# Copyright (c) 2013 The Chromium OS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

from fuzzy_check import FuzzyCheck
from validators import *

# originally generated gestures with box filter enabled:
#
# 15972.610917 - 15972.932469  |  Motion d=1.787 x=0 y=1.787 r=4.905e-06
# 15973.035596 - 15973.169617  |  Motion d=0.8936 x=0.2234 y=0.6702 r=2.776e-17
# 15973.370449 - 15973.380631  |  Motion d=0.2234 x=0 y=0.2234 r=2.776e-17
# 15973.523833 - 15973.534076  |  Motion d=0.2234 x=0 y=0.2234 r=2.776e-17
# 15973.844245 - 15973.864794  |  Motion d=0.4468 x=0.2234 y=0.2234 r=2.776e-17
# 15974.340741 - 15974.345890  |  Motion d=0.2234 x=0 y=0.2234 r=2.776e-17
# 15974.453534 - 15974.458708  |  Motion d=0.3159 x=0.2234 y=0.2234 r=0
# 15975.410640 - 15975.415788  |  Motion d=0.2234 x=0 y=0.2234 r=2.776e-17
# 15976.528128 - 15976.533288  |  Motion d=0.2234 x=0 y=0.2234 r=2.776e-17
# 15977.501757 - 15977.506925  |  Motion d=0.2234 x=0 y=0.2234 r=2.776e-17
# 15980.462388 - 15980.483873  |  Motion d=0.4468 x=0.4468 y=0 r=2.776e-17
#
# With stationary wiggle filter enabled and box filter disabled:
#
# 15972.610917 - 15972.676867  |  Motion d=0.6702 x=0 y=0.6702 r=4.188e-06
#
# which is the only motion from the instability of finger arrival

def Validate(raw, events, gestures):
  fuzzy = FuzzyCheck()
  fuzzy.expected = [
    MotionValidator("<2"),  # for the finger arrival
    MotionValidator("==0 ~ 5", merge=True),  # detect if there is any wiggling
  ]
  fuzzy.unexpected = [
    FlingStopValidator("<10"),
  ]
  return fuzzy.Check(gestures)
