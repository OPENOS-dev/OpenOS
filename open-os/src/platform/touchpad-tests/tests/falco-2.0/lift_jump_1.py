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
#   Motion d=215.8 x=194.1 y=71.45 r=1.792 s=304.4
#   Motion d=6.417 x=2.062 y=4.915 r=0.1388 s=13.15
#   Motion d=0.6416 x=0.3437 y=0.2979 r=0.03099 s=4.95
#   Motion d=14.81 x=0 y=14.81 r=4.335 s=687.6

def Validate(raw, events, gestures):
  fuzzy = FuzzyCheck()
  # Make sure no events after liftoff starts
  for event in events:
    dist = event.distance if hasattr(event, 'distance') else -1.0
    if float(event.end) > 40450.480275 and dist > 0.0:
      return False, 'Has jump movement at end'
  return 1, 'Success'
