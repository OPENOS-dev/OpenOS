# Copyright (c) 2012 The Chromium OS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# This module provides unit tests for FuzzyCheck from fuzzy_check.py including
# it's internal classes
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

from fuzzy_check import *
from validators import *
from gesture_log import *
import unittest

class FuzzyComparatorTests(unittest.TestCase):
  """
  Unit tests for FuzzyComparator. Tests the parsing various cases of input
  strings as well as the fuzzy score calculation when comparing to numbers.
  """

  def test_number_parsing(self):
    self.assertEqual(FuzzyComparator("100")._target, 100)
    self.assertEqual(FuzzyComparator("-100")._target, -100)
    self.assertEqual(FuzzyComparator("== -100")._target, -100)
    self.assertEqual(FuzzyComparator(">= -100~5")._target, -100)
    self.assertEqual(FuzzyComparator(">= -100 ~ 5")._target, -100)

  def test_operator_parsing(self):
    def opname(comp):
      return FuzzyComparator(comp)._operator.__name__
    self.assertEqual(opname("1"), "_CompareEq")
    self.assertEqual(opname("==-1~1"), "_CompareEq")
    self.assertEqual(opname(">= -1~1"), "_CompareGte")
    self.assertEqual(opname("<= -1~"), "_CompareLte")
    self.assertEqual(opname("> -1~1"), "_CompareGt")
    self.assertEqual(opname("< -1~"), "_CompareLt")

  def test_fuzzyness_parsing(self):
    self.assertEqual(FuzzyComparator("==-1~1")._fuzzyness, 1)
    self.assertEqual(FuzzyComparator(">= -1 ~ 99")._fuzzyness, 99)

  def test_sane_parsing_error(self):
    try:
      FuzzyComparator("-=1")
    except ValueError:
      return
    self.fail("Malformed comparator did not raise a ValueError")

  def test_eq(self):
    c = FuzzyComparator("== 100")
    self.assertEqual(c.Compare(100), 1)
    self.assertEqual(c.Compare(101), False)
    self.assertEqual(c.Compare(99), False)

  def test_fuzzy_eq(self):
    c = FuzzyComparator("== 100 ~ 10")
    self.assertAlmostEqual(c.Compare(100), 1)
    self.assertAlmostEqual(c.Compare(90), 0)
    self.assertAlmostEqual(c.Compare(110), 0)
    self.assertAlmostEqual(c.Compare(95), 0.5)
    self.assertAlmostEqual(c.Compare(105), 0.5)
    self.assertAlmostEqual(c.Compare(99), 0.9)
    self.assertAlmostEqual(c.Compare(101), 0.9)
    self.assertAlmostEqual(c.Compare(111), False)
    self.assertAlmostEqual(c.Compare(89), False)

  def test_lt_lte(self):
    c = FuzzyComparator("< 100")
    self.assertAlmostEqual(c.Compare(111), False)
    self.assertAlmostEqual(c.Compare(110), False)
    self.assertAlmostEqual(c.Compare(109), False)
    self.assertAlmostEqual(c.Compare(101), False)
    self.assertAlmostEqual(c.Compare(100), False)
    self.assertAlmostEqual(c.Compare(99) , 1)
    self.assertAlmostEqual(c.Compare(0)  , 1)

    c = FuzzyComparator("<= 100")
    self.assertAlmostEqual(c.Compare(111), False)
    self.assertAlmostEqual(c.Compare(110), False)
    self.assertAlmostEqual(c.Compare(109), False)
    self.assertAlmostEqual(c.Compare(101), False)
    self.assertAlmostEqual(c.Compare(100), 1)
    self.assertAlmostEqual(c.Compare(99) , 1)
    self.assertAlmostEqual(c.Compare(0)  , 1)

  def test_fuzzy_lt_lte(self):
    c = FuzzyComparator("< 100 ~ 10")
    self.assertAlmostEqual(c.Compare(111), False)
    self.assertAlmostEqual(c.Compare(110), 0)
    self.assertAlmostEqual(c.Compare(109), 0.1)
    self.assertAlmostEqual(c.Compare(101), 0.9)
    self.assertAlmostEqual(c.Compare(100), 1)
    self.assertAlmostEqual(c.Compare(99) , 1)
    self.assertAlmostEqual(c.Compare(0)  , 1)

    c = FuzzyComparator("<= 100 ~ 10")
    self.assertAlmostEqual(c.Compare(111), False)
    self.assertAlmostEqual(c.Compare(110), 0)
    self.assertAlmostEqual(c.Compare(109), 0.1)
    self.assertAlmostEqual(c.Compare(101), 0.9)
    self.assertAlmostEqual(c.Compare(100), 1)
    self.assertAlmostEqual(c.Compare(99) , 1)
    self.assertAlmostEqual(c.Compare(0)  , 1)

  def test_gt_gte(self):
    c = FuzzyComparator("> 100")
    self.assertAlmostEqual(c.Compare(99) , False)
    self.assertAlmostEqual(c.Compare(100), False)
    self.assertAlmostEqual(c.Compare(101), 1)
    self.assertAlmostEqual(c.Compare(999), 1)

    c = FuzzyComparator(">= 100")
    self.assertAlmostEqual(c.Compare(99) , False)
    self.assertAlmostEqual(c.Compare(100), 1)
    self.assertAlmostEqual(c.Compare(101), 1)
    self.assertAlmostEqual(c.Compare(999), 1)

  def test_fuzzy_gt_gte(self):
    c = FuzzyComparator("> 100 ~ 10")
    self.assertAlmostEqual(c.Compare(89) , False)
    self.assertAlmostEqual(c.Compare(90) , 0)
    self.assertAlmostEqual(c.Compare(91) , 0.1)
    self.assertAlmostEqual(c.Compare(99) , 0.9)
    self.assertAlmostEqual(c.Compare(100), 1)
    self.assertAlmostEqual(c.Compare(101), 1)
    self.assertAlmostEqual(c.Compare(999), 1)

    c = FuzzyComparator(">= 100 ~ 10")
    self.assertAlmostEqual(c.Compare(89) , False)
    self.assertAlmostEqual(c.Compare(90) , 0)
    self.assertAlmostEqual(c.Compare(91) , 0.1)
    self.assertAlmostEqual(c.Compare(99) , 0.9)
    self.assertAlmostEqual(c.Compare(100), 1)
    self.assertAlmostEqual(c.Compare(101), 1)
    self.assertAlmostEqual(c.Compare(999), 1)

  def test_fail_vs_zero_score(self):
    c = FuzzyComparator("== 100 ~ 10")

    self.assertTrue(c.Compare(110) == 0)

    self.assertFalse(c.Compare(110) is False)
    self.assertTrue(c.Compare(111) is False)

class FuzzyCheckTests(unittest.TestCase):
  """
  Unit tests for FuzzyCheck. All tests set up a set of validators and test
  the score when comparing to a list of gestures.
  """
  def setUp(self):
    self.fuzzy = FuzzyCheck()

  def test_empty_validator_list(self):
    self.assertEquals(self.fuzzy.Check([]), 1)

  def test_nothing_happened(self):
    self.fuzzy.expected = [ButtonDownValidator(1), ButtonUpValidator(1)]
    score = self.fuzzy.Check([])
    self.assertTrue(score is not False)
    self.assertEqual(score, 0)

  def test_motion_only(self):
    self.fuzzy.expected = [ButtonDownValidator(1), ButtonUpValidator(1)]
    self.fuzzy.unexpected = [MotionValidator("0~10")]
    score = self.fuzzy.Check([MotionGesture(0, 5, 0, 0)])
    self.assertTrue(score is not False)
    self.assertAlmostEqual(score, 0)

  def test_not_enough_Gestures(self):
    self.fuzzy.expected = [ButtonDownValidator(1), ButtonUpValidator(1)]
    score = self.fuzzy.Check([ButtonDownGesture(1, 0, 0)])
    self.assertAlmostEqual(score, 0)

  def test_unexpected_button_fail(self):
    self.fuzzy.expected = [MotionValidator(100)]
    score = self.fuzzy.Check([MotionGesture(0, 100, 0, 0),
                              ButtonDownGesture(1, 0, 0)])
    self.assertTrue(score is False)

  def test_unexpected_motion_accumulation_fail(self):
    self.fuzzy.expected = [ButtonDownValidator(1), ButtonUpValidator(1)]
    self.fuzzy.unexpected = [MotionValidator("<5")]
    score = self.fuzzy.Check([MotionGesture(0, 4, 0, 0),
                              ButtonDownGesture(1, 0, 0),
                              MotionGesture(0, 4, 0, 0),
                              ButtonUpGesture(1, 0, 0)])
    self.assertTrue(score is False)

  def test_unexpected_motion_accumulation_success(self):
    self.fuzzy.expected = [ButtonDownValidator(1), ButtonUpValidator(1)]
    self.fuzzy.unexpected = [MotionValidator("<10")]
    score = self.fuzzy.Check([MotionGesture(0, 4, 0, 0),
                              ButtonDownGesture(1, 0, 0),
                              MotionGesture(0, 4, 0, 0),
                              ButtonUpGesture(1, 0, 0)])
    self.assertEqual(score, 1)

  def test_anything_but_success(self):
    self.fuzzy.expected = [AnythingButValidator(ButtonDownValidator(1)),
                           ButtonDownValidator(1)]
    score = self.fuzzy.Check([ScrollGesture(0, 1, 0, 0),
                              ButtonUpGesture(1, 0, 0),
                              MotionGesture(0, 10, 0, 0),
                              ButtonDownGesture(1, 0, 0)])
    self.assertEqual(score, 1)

  def test_x_validation_success(self):
    self.fuzzy.expected = [MotionValidator(x="10")]
    self.assertAlmostEqual(self.fuzzy.Check([MotionGesture(10, 0, 0, 0)]), 1)

  def test_x_validation_fail(self):
    self.fuzzy.expected = [MotionValidator(x="10")]
    # note: 'is' compares the object id. Only a reference to False will pass
    self.assertTrue(self.fuzzy.Check([MotionGesture(0, 10, 0, 0)]) is False)

  def test_y_validation_success(self):
    self.fuzzy.expected = [MotionValidator(y="10")]
    self.assertAlmostEqual(self.fuzzy.Check([MotionGesture(0, 10, 0, 0)]), 1)

  def test_y_validation_fail(self):
    self.fuzzy.expected = [MotionValidator(y="10")]
    # note: 'is' compares the object id. Only a reference to False will pass
    self.assertTrue(self.fuzzy.Check([MotionGesture(10, 0, 0, 0)]) is False)

  def test_start_validation_success(self):
    self.fuzzy.expected = [MotionValidator("10", start="<10")]
    self.assertAlmostEqual(self.fuzzy.Check([MotionGesture(10, 0, 5, 0)]), 1)

  def test_start_validation_fail(self):
    self.fuzzy.expected = [MotionValidator("10", start="<10")]
    # note: 'is' compares the object id. Only a reference to False will pass
    self.assertTrue(self.fuzzy.Check([MotionGesture(10, 0, 20, 0)]) is False)

  def test_end_validation_success(self):
    self.fuzzy.expected = [MotionValidator("10", end="<10")]
    self.assertAlmostEqual(self.fuzzy.Check([MotionGesture(10, 0, 0, 5)]), 1)

  def test_end_validation_fail(self):
    self.fuzzy.expected = [MotionValidator("10", end="<10")]
    # note: 'is' compares the object id. Only a reference to False will pass
    self.assertTrue(self.fuzzy.Check([MotionGesture(10, 0, 0, 20)]) is False)

  def test_time_distance_combination_score(self):
    self.fuzzy.expected = [MotionValidator("100~10", start=">10~10",
                                           end="<50~10")]

    self.assertAlmostEqual(self.fuzzy.Check([MotionGesture(105, 0, 5, 50)]),
                           0.25)

  def test_fling_and_fling_stop(self):
    self.fuzzy.expected = [FlingValidator("100")]
    self.fuzzy.unexpected = [FlingStopValidator()]
    score = self.fuzzy.Check([FlingGesture(0, 50, 0, 0),
                              FlingStopGesture(0, 0),
                              FlingGesture(0, 50, 0, 0)])
    self.assertAlmostEqual(score, 1)


class FuzzyCheckDragTests(unittest.TestCase):
  """
  Unit tests for FuzzyCheck. A complete test of FuzzyCheck for one use case:
  FuzzyCheck is set up to detect a drag gesture (Button down, move, button up)
  and then used to validate various lists of gestures. The resulting score of
  this validation is then checked by the unit tests in this class.
  """

  def setUp(self):
    self.fuzzy = FuzzyCheck()
    self.fuzzy.expected = [
      ButtonDownValidator(1),
      MotionValidator("== 100 ~ 10"),
      ButtonUpValidator(1)
    ]
    self.fuzzy.unexpected = [MotionValidator("<= 10 ~ 10")]

  def test_perfect_success(self):
    self.fuzzy.unexpected = [MotionValidator("== 0")]
    Gestures = [
      ButtonDownGesture(1, 0, 0),
      MotionGesture(0, 100, 0, 0),
      ButtonUpGesture(1, 0, 0)
    ]
    score = self.fuzzy.Check(Gestures)
    self.assertTrue(score is not False)
    self.assertEqual(score, 1)

  def test_scored_success(self):
    Gestures = [
      ButtonDownGesture(1, 0, 0),
      MotionGesture(0, 105, 0, 0),
      ButtonUpGesture(1, 0, 0)
    ]
    score = self.fuzzy.Check(Gestures)
    self.assertTrue(score is not False)
    self.assertEqual(score, 0.5)

  def test_unexpected_motion_perfect_success(self):
    Gestures = [
      MotionGesture(0, 5, 0, 0),
      ButtonDownGesture(1, 0, 0),
      MotionGesture(0, 100, 0, 0),
      ButtonUpGesture(1, 0, 0),
      MotionGesture(0, 5, 0, 0)
    ]

    score = self.fuzzy.Check(Gestures)
    self.assertTrue(score is not False)
    self.assertEqual(score, 1)

  def test_unexpected_motion_scored_success(self):
    Gestures = [
      MotionGesture(0, 10, 0, 0),
      ButtonDownGesture(1, 0, 0),
      MotionGesture(0, 100, 0, 0),
      ButtonUpGesture(1, 0, 0),
      MotionGesture(0, 5, 0, 0)
    ]

    score = self.fuzzy.Check(Gestures)
    self.assertTrue(score is not False)
    self.assertEqual(score, 0.5)

  def test_motion_failure(self):
    Gestures = [
      ButtonDownGesture(1, 0, 0),
      MotionGesture(0, 120, 0, 0),
      ButtonUpGesture(1, 0, 0)
    ]
    score = self.fuzzy.Check(Gestures)
    self.assertTrue(score is False)

  def test_unexpected_motion_failure(self):
    Gestures = [
      MotionGesture(0, 20, 0, 0),
      ButtonDownGesture(1, 0, 0),
      MotionGesture(0, 100, 0, 0),
      ButtonUpGesture(1, 0, 0),
      MotionGesture(0, 20, 0, 0)
    ]

    score = self.fuzzy.Check(Gestures)
    self.assertTrue(score is False)


if __name__ == '__main__':
  unittest.main()
