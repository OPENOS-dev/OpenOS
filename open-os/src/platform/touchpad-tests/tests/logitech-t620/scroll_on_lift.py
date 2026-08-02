# Copyright (c) 2013 The Chromium OS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

from fuzzy_check import FuzzyCheck
from validators import *

# originally generated gestures:
#

def Validate(raw, events, gestures):
  fuzzy = FuzzyCheck()
  for rawitem in raw['entries']:
    if rawitem['type'] == 'gesture' and rawitem['gestureType'] == 'scroll':
      dy = rawitem['dy']
      if dy > 0:
        return False, 'Has reverse scroll'
  return True, 'Success'
