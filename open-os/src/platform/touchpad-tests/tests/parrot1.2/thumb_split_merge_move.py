# Copyright (c) 2012 The Chromium OS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

from fuzzy_check import FuzzyCheck
from validators import *

# originally generated gestures:
# FlingStop
#   ButtonDown(4)
#   ButtonUp(4)
#   ButtonDown(1)
#   ButtonUp(1)

# This is a case about thumb click casues right click. Thumb splitting happens
# and then one of the splitted finger moves and the merged finger is unmerged.
# We want to set the "Split Merge Max Movement" value large enought to make
# sure the unmerge does not happen.

def Validate(raw, events, gestures):
  fuzzy = FuzzyCheck()
  fuzzy.expected = [
    # Make sure no right click happens.
    ButtonDownValidator(1),
    ButtonUpValidator(1),
    ButtonDownValidator(1),
    ButtonUpValidator(1),
  ]
  fuzzy.unexpected = [
    FlingStopValidator("<10"),
  ]
  return fuzzy.Check(gestures)
