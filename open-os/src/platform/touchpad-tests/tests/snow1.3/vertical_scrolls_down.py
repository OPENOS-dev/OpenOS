# Copyright (c) 2012 The Chromium OS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

from fuzzy_check import FuzzyCheck
from validators import *


def Validate(raw, events, gestures):
  fuzzy = FuzzyCheck()
  fuzzy.expected = [
    ScrollValidator("> 1000.0 ~ 500.0"),
    FlingValidator("> 5000.0 ~ 2500.0"),
    ScrollValidator("> 833.3 ~ 416.7"),
    FlingValidator("> 500.0 ~ 250.0"),
    ScrollValidator("> 166.7 ~ 83.3"),
    FlingValidator("0"),
    ScrollValidator("> 750.0 ~ 375.0"),
    FlingValidator("> 2500.0 ~ 1250.0"),
  ]
  fuzzy.unexpected = [
    FlingStopValidator("<10"),
  ]
  return fuzzy.Check(gestures)
