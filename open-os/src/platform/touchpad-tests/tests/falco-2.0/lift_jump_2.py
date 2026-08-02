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
#   Motion d=271.2 x=196.4 y=142.5 r=1.131 s=268.7

def Validate(raw, events, gestures):
  fuzzy = FuzzyCheck()
  # Make sure no events after liftoff starts
  for event in events:
    dist = event.distance if hasattr(event, 'distance') else -1.0
    if float(event.end) > 40816.322409 and dist > 0.0:
      return False, 'Has jump movement at end'
  return 1, 'Success'
