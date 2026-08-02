# Copyright (c) 2014 The Chromium OS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

from fuzzy_check import FuzzyCheck
from validators import *

# originally generated gestures:
# FlingStop
#   Swipe d=894.7 x=0 y=894.7 r=5.3 s=3702
#   SwipeLift

def Validate(raw, events, gestures):
  dy = 0
  for entry in raw['entries']:
    if (entry['type'] != 'gesture' or
        entry['gestureType'] != 'swipe'):
      continue
    dy = dy + entry['dy']
  if dy < 0:
    return True, 'Success'
  return False, 'dy should be less than 0'
