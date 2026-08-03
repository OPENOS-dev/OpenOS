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
    The feedback report showed a pinch gesture where only motion should
    have happened. This test is passes as long as there is no
    pinch gesture happening.
  """
  fuzzy = FuzzyCheck()
  fuzzy.expected = [
    AnythingButValidator(PinchValidator())
  ]
  fuzzy.unexpected = [
    AnythingButValidator(PinchValidator())
  ]
  return fuzzy.Check(gestures)
