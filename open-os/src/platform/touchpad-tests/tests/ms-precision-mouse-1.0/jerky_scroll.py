# Copyright 2020 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

from fuzzy_check import FuzzyCheck
from validators import *

# Upon BT wakeup, multiple BT packets may be received back to back
# with a smaller than expected time delta from the perspective of the
# system, resulting in arbitrarily large amounts of acceleration.
# We clipped that time delta to a minimum value, and this test is to
# validate that fix.

def Validate(raw, events, gestures):
  fuzzy = FuzzyCheck()
  fuzzy.expected = [
    ScrollValidator("== 169 ~10"),
    ScrollValidator("== 403 ~10"),
  ]
  fuzzy.unexpected = [
  ]
  return fuzzy.Check(gestures)
