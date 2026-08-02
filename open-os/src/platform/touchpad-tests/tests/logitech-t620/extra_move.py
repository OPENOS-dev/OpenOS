# Copyright (c) 2013 The Chromium OS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

from fuzzy_check import FuzzyCheck
from validators import *
import inspect

# originally generated gestures:
# Motion d=1370 x=1351 y=218.7 r=12.16 s=1403
#   FlingStop

def Validate(raw, events, gestures):
  left_motion = 0
  right_motion = 0
  for rawitem in raw['entries']:
    if rawitem['type'] == 'gesture' and rawitem['gestureType'] == 'move':
      dx = float(rawitem['dx'])
      if dx > 0:
        right_motion = right_motion + dx
      else:
        left_motion = left_motion - dx
  # Left and right motion must differ by less than 15%
  delta = math.fabs(left_motion - right_motion)
  avg = (left_motion + right_motion) / 2.0
  if delta > avg * 0.15:
    return False, 'Left and right motion differ too much'
  return True, 'Success'
