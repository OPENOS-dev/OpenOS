# Copyright (c) 2012 The Chromium OS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

from fuzzy_check import FuzzyCheck
from validators import *

# This tests that even when a person hammers down on the touchpad so quickly
# that the physical click occurs *before* the touchpad senses the finger, that
# clicks are still registered.

# originally generated gestures:
# ButtonDown(4)
# ButtonUp(4)
# FlingStop
# ButtonDown(4)
# ButtonUp(4)
# FlingStop
# ButtonDown(4)
# ButtonUp(4)
# FlingStop
# ButtonDown(4)
# ButtonUp(4)
# FlingStop
# ButtonDown(4)
# ButtonUp(4)
# FlingStop
# ButtonDown(1)
# ButtonUp(1)
# FlingStop
# ButtonDown(1)
# ButtonUp(1)
# FlingStop
# ButtonDown(1)
# ButtonUp(1)
# FlingStop
# ButtonDown(1)
# ButtonUp(1)
# FlingStop
# ButtonDown(1)
# ButtonUp(1)
# FlingStop
# ButtonDown(1)
# ButtonUp(1)
# FlingStop

def Validate(raw, events, gestures):
  fuzzy = FuzzyCheck()
  fuzzy.expected = [
    ButtonDownValidator(4),
    ButtonUpValidator(4),
    ButtonDownValidator(4),
    ButtonUpValidator(4),
    ButtonDownValidator(4),
    ButtonUpValidator(4),
    ButtonDownValidator(4),
    ButtonUpValidator(4),
    ButtonDownValidator(4),
    ButtonUpValidator(4),
    ButtonDownValidator(1),
    ButtonUpValidator(1),
    ButtonDownValidator(1),
    ButtonUpValidator(1),
    ButtonDownValidator(1),
    ButtonUpValidator(1),
    ButtonDownValidator(1),
    ButtonUpValidator(1),
    ButtonDownValidator(1),
    ButtonUpValidator(1),
    ButtonDownValidator(1),
    ButtonUpValidator(1),
    ButtonDownValidator(1),
    ButtonUpValidator(1),
  ]
  fuzzy.unexpected = [
    FlingStopValidator("<15"),
  ]
  return fuzzy.Check(gestures)
