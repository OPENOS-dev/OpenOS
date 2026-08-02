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
#   Scroll d=1288 x=0 y=1288 r=67.46 s=1.079e+04

def Validate(raw, events, gestures):
  for entry in raw['entries']:
    if entry['type'] == 'gesture':
      if entry['endTime'] > 8638.763849:
        return False, "Has activity during lift"
  return True, "Success"
