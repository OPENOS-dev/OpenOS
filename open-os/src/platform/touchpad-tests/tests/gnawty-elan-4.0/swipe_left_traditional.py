# Copyright (c) 2014 The Chromium OS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

from fuzzy_check import FuzzyCheck
from validators import *

# originally generated gestures:
# Swipe d=369.8 x=369.8 y=0 r=1.493 s=1063
#   FlingStop
#   SwipeLift

def Validate(raw, events, gestures):
  dx = 0
  for entry in raw['entries']:
    if (entry['type'] != 'gesture' or
        entry['gestureType'] != 'swipe'):
      continue
    dx = dx + entry['dx']
  if dx < 0:
    return True, 'Success'
  return False, 'dx should be less than 0'
