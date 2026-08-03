# Copyright (c) 2012 The Chromium OS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

from fuzzy_check import FuzzyCheck
from validators import *


def Validate(raw, events, gestures):
  """
    Feedback showed the cursor jumping after lifting the finger.
    Make sure it does not happen.
  """
  fuzzy = FuzzyCheck()
  # Make sure no events after 4924.525335 occur
  for event in events:
    dist = event.distance if hasattr(event, 'distance') else -1.0
    if float(event.end) > 4924.525 and dist > 0.0:
      return False, 'Has jump movement at end'
  return 1, 'Success'
