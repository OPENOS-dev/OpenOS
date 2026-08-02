# Copyright (c) 2013 The Chromium OS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

from fuzzy_check import FuzzyCheck
from validators import *

# originally generated gestures:
# FlingStop
#   Motion d=60.81 x=7.308 y=56.26 r=0.5693 s=118.5

def Validate(raw, events, gestures):
  fuzzy = FuzzyCheck()
  # Make sure no events after liftoff starts
  for event in events:
    dist = event.distance if hasattr(event, 'distance') else -1.0
    if float(event.end) > 41623.151737 and dist > 0.0:
      return False, 'Has jump movement at end'
  return 1, 'Success'
