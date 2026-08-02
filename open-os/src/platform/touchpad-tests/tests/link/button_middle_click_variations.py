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
    Test if different variations of performing a middle click using
    the phyiscal button is working.
  """
  fuzzy = FuzzyCheck()
  fuzzy.expected = [
    ButtonDownValidator(2),
    ButtonUpValidator(2),
    ButtonDownValidator(2),
    ButtonUpValidator(2),
    ButtonDownValidator(2),
    ButtonUpValidator(2),
    ButtonDownValidator(2),
    ButtonUpValidator(2),
    # the expected behavior of this click changed:
    # 2 cold fingers plus the touchdown of a 3rd finger that clicks the button
    # is no longer supposed to do a middle click, but a left click
    ButtonDownValidator(1),
    ButtonUpValidator(1),
    ButtonDownValidator(2),
    ButtonUpValidator(2),
    ButtonDownValidator(2),
    ButtonUpValidator(2)
  ]
  fuzzy.unexpected = [
    MotionValidator("<10"),
    FlingStopValidator("<20"),
  ]
  return fuzzy.Check(gestures)
