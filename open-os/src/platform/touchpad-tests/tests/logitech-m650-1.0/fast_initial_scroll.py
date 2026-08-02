# Copyright 2024 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from fuzzy_check import FuzzyCheck
from validators import *

# Upon BT wakeup, multiple BT packets may be received back to back
# with a smaller than expected time delta from the perspective of the
# system, resulting in arbitrarily large amounts of acceleration.
# To remove unexpectedly large scrolls in the scenario without slowing
# down intentionally fast scroll, we now use several scroll events when
# calculating scroll velocity.
#
# The first scroll is an example of a very fast scroll on BT wakeup,
# and the second scroll is a normal speed scroll.

def Validate(raw, events, gestures):
  fuzzy = FuzzyCheck()
  fuzzy.expected = [
    ScrollValidator("== 41 ~ 20"),
    ScrollValidator("== 30 ~ 20"),
  ]
  fuzzy.unexpected = [
  ]
  return fuzzy.Check(gestures)
