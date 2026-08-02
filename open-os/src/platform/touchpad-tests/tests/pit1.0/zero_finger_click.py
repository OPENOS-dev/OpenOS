# Copyright (c) 2013 The Chromium OS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

from fuzzy_check import FuzzyCheck
from validators import *

# originally generated gestures:
#    ButtonDown(1)
#    ButtonUp(1)
#    ButtonDown(1)
#    ButtonUp(1)
#    ButtonDown(1)
#    ButtonUp(1)
#    ButtonDown(1)
#    ButtonUp(1)

def Validate(raw, events, gestures):
  fuzzy = FuzzyCheck()
  fuzzy.expected = [ ]
  fuzzy.unexpected = [
    FlingStopValidator("<10")
  ]
  return fuzzy.Check(gestures)
