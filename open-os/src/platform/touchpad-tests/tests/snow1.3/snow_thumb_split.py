# Copyright (c) 2012 The Chromium OS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

from fuzzy_check import FuzzyCheck
from validators import *

# The default setting of SplitCorrectingFilter will generate a right click
# for this thumb click. We want a setting that can correct this as a thumb
# click.
# originally generated gestures:
# ButtonDown(4)
#   FlingStop
#   Motion d=3 x=3 y=0 r=0.00
#   ButtonUp(4)

def Validate(raw, events, gestures):
  fuzzy = FuzzyCheck()
  fuzzy.expected = [
     ButtonDownValidator(1),
     ButtonUpValidator(1),
  ]
  fuzzy.unexpected = [
     MotionValidator("<10"),
     FlingStopValidator("<10"),
  ]
  return fuzzy.Check(gestures)
