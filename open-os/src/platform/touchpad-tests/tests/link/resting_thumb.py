# Copyright (c) 2012 The Chromium OS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

from fuzzy_check import FuzzyCheck
import mtlib.gesture_log
from validators import *

# originally generated gestures:
# FlingStop
#   Motion d=497 x=494 y=39 r=0.12
#   ButtonDown(1)
#   ButtonUp(1)
#   ButtonDown(1)
#   ButtonUp(1)
#   FlingStop
#   Motion d=229 x=25 y=227 r=0.13
#   FlingStop
#   Motion d=133 x=22 y=130 r=0.11
#   FlingStop
#   Motion d=156 x=155 y=6 r=4.23
#   Pinch dz=88 r=0.00

def Validate(raw, events, gestures):
  # We want to make sure that move gestures only occur when a non-thumb
  # is on the pad

  # Compute start/end times of all non-thumb fingers
  thumb_id = 104
  finger_times = {}
  def noteFinger(finger_state, timestamp):
    tr_id = finger_state['trackingId']
    if tr_id is thumb_id:
      return
    if tr_id not in finger_times:
      finger_times[tr_id] = [timestamp, timestamp]
      return
    finger_times[tr_id][0] = min(finger_times[tr_id][0], timestamp)
    finger_times[tr_id][1] = max(finger_times[tr_id][1], timestamp)
  def isValidFingerTime(timestamp):
    for finger in finger_times:
      start = finger_times[finger][0]
      end = finger_times[finger][1]
      if timestamp >= start and timestamp <= end:
        return True
    return False

  for entry in raw['entries']:
    if entry['type'] != 'hardwareState':
      continue
    for finger in entry['fingers']:
      noteFinger(finger, entry['timestamp'])

  for gesture in events:
    if not isinstance(gesture, mtlib.gesture_log.MotionGesture):
      continue
    if not (isValidFingerTime(gesture.start) and
            isValidFingerTime(gesture.end)):
      return False, 'Invalid finger time'
  return True, 'Success'
