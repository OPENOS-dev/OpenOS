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
#   Scroll d=817 x=5 y=812 r=8.754 s=2269
#   Scroll d=17 x=0 y=17 r=1.493 s=659.1
#   Fling d=0 x=0 y=0 r=0 s=0

def Validate(raw, events, gestures):
  fuzzy = FuzzyCheck()
  for rawitem in raw['entries']:
    if rawitem['type'] == 'gesture' and rawitem['gestureType'] == 'scroll':
      dy = rawitem['dy']
      if dy > 0:
        return False, 'Has reverse scroll'
  return True, 'Success'
