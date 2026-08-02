# Copyright (c) 2012 The Chromium OS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# This module contains unit tests for the GestureLog class
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

from os import path
import random
import unittest

from gesture_log import GestureLog
from gesture_log import MotionGesture

class GestureLogTests(unittest.TestCase):
  """
  Unit tests for GestureLog. These tests open one of the activity log files.
  The events from these files will be condensed to gestures and the tests
  in this class will check if the right number of those gestures have been
  generated. Each continuous motion on the trackpad should result in one
  gesture.
  """
  def _log_from_test_file(self, suffix):
    filename = path.splitext(__file__)[0] + "_" + suffix + ".txt"
    data = open(filename, "r").read()
    return GestureLog(data)

  def test_fast_movement_log(self):
    log = self._log_from_test_file("4_move_fast")
    motions = filter(lambda g: g.type == "Motion", log.gestures)
    self.assertEqual(len(motions), 4)

  def test_movement_log(self):
    log = self._log_from_test_file("4_move")
    motions = filter(lambda g: g.type == "Motion", log.gestures)
    self.assertEqual(len(motions), 4)

  def test_scroll_log(self):
    log = self._log_from_test_file("4_scrolls")
    motions = filter(lambda g: g.type == "Scroll", log.gestures)
    self.assertEqual(len(motions), 4)

  def test_move_scroll_log(self):
    log = self._log_from_test_file("4_move+scrolls")
    gestures = filter(lambda g: g.type != "FlingStop", log.gestures)
    self.assertEqual(len(gestures), 4)
    for i in range(0, 4):
      if i % 2:
        self.assertEqual(gestures[i].type, "Scroll")
      else:
        self.assertEqual(gestures[i].type, "Motion")

  def test_motion_merge_with_tap_and_half_log(self):
    log = self._log_from_test_file("4_tap_and_half")
    motions = filter(lambda g: g.type == "Motion", log.gestures)
    self.assertEqual(len(motions), 4)

  def test_clicks_log(self):
    log = self._log_from_test_file("4_clicks")
    gestures = filter(lambda g: g.type != "FlingStop", log.gestures)
    self.assertEqual(len(gestures), 4)
    for i in range(0, 4):
      if i % 2:
        self.assertEqual(gestures[i].type, "ButtonUp")
      else:
        self.assertEqual(gestures[i].type, "ButtonDown")

  def test_fling_stop_log(self):
    log = self._log_from_test_file("4_flings+stops")
    gestures = filter(lambda g: g.type != "FlingStop", log.gestures)
    for i in range(0, 8):
      if i % 2 == 0:
        self.assertEqual(gestures[i].type, "Scroll")
      else:
        self.assertEqual(gestures[i].type, "Fling")


class GestureLogRoughnessTests(unittest.TestCase):
  """
  Unit tests for GestureLog. These tests will check the behavior of the
  roughness measure by providing different motions.
  """
  def test_zero_roughness(self):
    """
    Constant motion has to result in 0 roughness
    """
    motion = MotionGesture(10, 10, 0, 0)
    for i in range(0, 10):
      motion.Append(MotionGesture(10, 10, 0, 0))
    self.assertAlmostEqual(motion.Roughness(), 0)

  def test_low_roughness(self):
    """
    Generate motion events with movement distances between 9 and 11.
    This is a very low deviation which should result in a low roughness.
    As this test is generated with random numbers, repeat the test 1000 times.
    """
    for j in range(0, 1000):
      motion = MotionGesture(10, 10, 0, 0)
      for i in range(0, 10):
        dist = random.randint(9, 11)
        motion.Append(MotionGesture(dist, dist, 0, 0))
      self.assertTrue(motion.Roughness() < 2.0)

  def test_high_roughness(self):
    """
    One case with high roughness is when the movement distances jump between
    high and low values (0 and 20 in this case).
    """
    motion = MotionGesture(10, 10, 0, 0)
    for i in range(0, 10):
      if i % 2:
        motion.Append(MotionGesture(0, 0, 0, 0))
      else:
        motion.Append(MotionGesture(20, 20, 0, 0))
    self.assertTrue(motion.Roughness() > 10.0)

  def test_roughness_peak(self):
    """
    Another case which leads to high roughness is when the motion distances
    contain an outliner.
    """
    motion = MotionGesture(10, 10, 0, 0)
    for i in range(0, 20):
      if i == 10:
        motion.Append(MotionGesture(100, 100, 0, 0))
      else:
        motion.Append(MotionGesture(10, 10, 0, 0))
    self.assertTrue(motion.Roughness() > 10.0)

  def test_roughness_jump(self):
    """
    Another case which leads to high roughness is when the motion distances
    contain a sudden jump in values.
    """
    motion = MotionGesture(10, 10, 0, 0)
    for i in range(0, 20):
      if i > 10:
        motion.Append(MotionGesture(100, 100, 0, 0))
      else:
        motion.Append(MotionGesture(10, 10, 0, 0))
    self.assertTrue(motion.Roughness() > 10.0)

if __name__ == '__main__':
  unittest.main()
