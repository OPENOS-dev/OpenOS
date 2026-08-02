# Copyright 2017 The Chromium OS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

from fuzzy_check import FuzzyCheck
from validators import *

# Palm detected as two contacts along the edge with a pinchlike motion. No
# gesture should be detected.

def Validate(raw, events, gestures):
  fuzzy = FuzzyCheck()
  fuzzy.expected = [
      AnythingButValidator(PinchValidator()),
  ]
  fuzzy.unexpected = [
  ]
  return fuzzy.Check(gestures)
