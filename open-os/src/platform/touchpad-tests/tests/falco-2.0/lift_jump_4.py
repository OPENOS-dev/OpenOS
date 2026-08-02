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
#   Motion d=456.1 x=325.7 y=249.8 r=2.194 s=358.3

def Validate(raw, events, gestures):
  fuzzy = FuzzyCheck()
  # Make sure no events after liftoff starts
  for event in events:
    dist = event.distance if hasattr(event, 'distance') else -1.0
    if float(event.end) > 41640.796483 and dist > 0.0:
      return False, 'Has jump movement at end'
  return 1, 'Success'
