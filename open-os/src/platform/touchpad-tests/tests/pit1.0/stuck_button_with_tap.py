# Copyright (c) 2014 The Chromium OS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function


import mtlib

from fuzzy_check import FuzzyCheck
from validators import *

# originally generated gestures:
# FlingStop
#   ButtonDown(1)
#   ButtonUp(1)

def Validate(raw, events, gestures):
  # This log was recorded from the button getting pressed down without a finger
  # on the pad, then a real finger taps, before the button is released.
  # The test should pass if only the tap is registered, the button should be
  # ignored entirely, so we check for a ButtonDown and ButtonUp that occur at
  # the same time (tap) and reject if there is a time gap (button press)

  down_event_type = mtlib.gesture_log.ButtonDownGesture
  up_event_type = mtlib.gesture_log.ButtonUpGesture

  down_events = [e for e in events if type(e) == down_event_type]
  up_events = [e for e in events if type(e) == up_event_type]

  # First check that there is only 1 click/tap seen (not both)
  if (len(down_events) != 1 or len(up_events) != 1):
    return 0.0, 'Too many button press events!'

  # Next, measure the time delta between the down and the up
  start_delta = down_events[0].start - up_events[0].start
  end_delta = down_events[0].end - up_events[0].end

  if (start_delta != 0.0 or end_delta != 0.0):
    return 0.0, 'Button events occured too far apart!'

  return 1.0, 'Success'
