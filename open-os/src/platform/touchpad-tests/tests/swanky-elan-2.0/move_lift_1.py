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
#   Motion d=139.2 x=106.7 y=86.72 r=0.3793 s=317.5

def Validate(raw, events, gestures):
  for entry in raw['entries']:
    if entry['type'] == 'gesture':
      if entry['endTime'] > 7334.241601:
        return False, "Has activity during lift"
  return True, "Success"
