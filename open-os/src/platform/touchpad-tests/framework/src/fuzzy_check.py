# Copyright (c) 2012 The Chromium OS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# This module contains the FuzzyCheck class that uses the validators
# defined in the validators modules. The validators make
# use of the FuzzyComparator class that allows validations to be made in
# a fuzzy way that leads to a score instead of a true/false decision.
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import math
import re

from table import Table

class FuzzyCheck(object):
  """
  This class can validate a list of gestures to a previously defined set of
  validators. This is used to check if the generated gestures actually
  represent the desired behavior.
  To allow more fine-grained results the output is a score instead of a
  match/no-match decision.
  There are two types of validators: expected or unexpected.
  Expected validators describe the expected behavior in the order it should
  happen. Whenever one of these validators cannot handle a gesture it is
  treated as unexpected and checked by the unexpected validators. A common
  case is expecting a ButtonClick, but allowing a certain amount of
  unexpected Motion to happen.
  """
  def __init__(self):
    self.expected = []
    self.unexpected = []

  def Check(self, event_list):
    """
    Validate gestures against the validators of this object. The expected
    and unexpected variables have to be set up before calling this method.
    This method will return a score between 0..1. It might also return False
    which is different from 0. False describes a complete test-failure, i.e.
    something has happened that should not have happened.
    When it comes to usability impact, it is better when nothing happens, as if
    something completely wrong happens. This is why 'nothing' happened is rated
    slightly better (score=0) as a non-matching behavior (score=False).
    """

    # special case for empty event list
    if len(event_list) == 0:
      if len(self.expected) == 0:
        return 1, "success, no events expected"
      else:
        return 0, "failure, expected events but nothing happened"

    # select first validator
    current_validator = None
    validators = list(reversed(self.expected))
    if len(self.expected) > 0:
      current_validator = validators.pop()

    table = Table("    ")
    table.title = "Validation Log"
    table.header("start - end", "event", "validator (* unexpected)")

    def AddRow(event, validator, unexpected=False):
      if unexpected:
         validator = "*" + str(validator)
      table.row("%f - %f" % (event.start, event.end), event, validator)

    def CheckUnexpectedEvent(event):
      for validator in self.unexpected:
        if validator.Accept(event):
          validator.Validate(event)
          AddRow(event, validator, True)
          return True
      # no validator found. Fail.
      AddRow(event, "no matching validator")
      return False

    failed = False
    for event in event_list:
      if failed:
        AddRow(event, "-")
        continue

      if current_validator is None:
        # No validator left. All events unexpected.
        if not CheckUnexpectedEvent(event):
          failed = True
      else:
        if not current_validator.Accept(event):
          if (current_validator.score > 0 and len(validators) > 0
              and validators[-1].Accept(event)):
              # transition to next validator
              current_validator = validators.pop()
              current_validator.Validate(event)
              AddRow(event, current_validator)
          else:
            # unexpected event
            if not CheckUnexpectedEvent(event):
              failed = True
        else:
          # accepted by current validator
          current_validator.Validate(event)
          AddRow(event, current_validator)

    score_table = self._BuildScoreTable(False)
    report = str(table) + "\n" + str(score_table)

    # calculate score
    if failed:
      score = False
    else:
      score = 1

      # expected validators score
      for validator in self.expected:
        if validator.score is False:
          return False, report
        else:
          score = score * validator.score

      # unexpected validators score
      for validator in self.unexpected:
        if validator.score is False:
          return False, report
        else:
          score = score * validator.score

    score_table = self._BuildScoreTable(score)
    report = str(table) + "\n" + str(score_table)

    return score, report

  def _BuildScoreTable(self, overall):
    table = Table("    ")
    table.title = "Score Breakdown"

    table.header("expected validator", "score")
    for validator in self.expected:
      table.row(validator, validator.score)


    table.header("unexpected validator", "score")
    for validator in self.unexpected:
      table.row(validator, validator.score)

    table.footer = "overall score: " + str(overall)
    return table


################################################################################
# Fuzzy Comparator
################################################################################

class FuzzyComparator(object):
  """
  This class is able to calculate fuzzy comparisons to return a scoring
  value. Fuzzy comparisons are inequality/equality operators with
  an addition range of validity. The result of the comparison is not
  a true/false decision but a score based on the range of validity.
  The comparison string format is:
    "[op] value [ ~ range ]"
  With the following options for op: ==, >, >=, <, <=
  Value being the integer value we Compare to.
  The range of validity is: [value-range .. value+range]
  If op is omitted we assume op to be == and if range is omitted
  it is assumed to be 0.
  See FuzzyComparator.Compare for information about the return value of
  comparisons.
  Example:
    fuzzy = FuzzyComparator("== 100 ~10")
    fuzzy.Compare(100) -> 1
    fuzzy.Compare(105) -> 0.5
    fuzzy.Compare(90)  -> 0
    fuzzy.Compare(89)  -> False
  """
  def __init__(self, input):
    if isinstance(input, str):
      m = re.match("([><=]=?)?[ ]*(\\-?[0-9]+\\.?[0-9]*)[ ]*" +
                   "(~[ ]*[0-9]+\\.?[0-9]*)?", input)

      if not m:
        raise ValueError("Malformed comparator: '" + input + "'")

      operator = m.group(1)
      value = m.group(2)
      fuzzyness = m.group(3)

      if value is None:
        raise ValueError("Malformed comparator: '" + input + "'")

      self._target = float(value)

      if operator is not None:
        self._operator_str = m.group(1)
      else:
        self._operator_str = "=="

      self._operator = FuzzyComparator._operator_mapping[self._operator_str]
      if fuzzyness is not None:
        self._fuzzyness = float(fuzzyness[1:])
      else:
        self._fuzzyness = 0
    else:
      self._target = float(input)
      self._fuzzyness = 0
      self._operator = FuzzyComparator._CompareEq
      self._operator_str = "=="

  def __str__(self):
    result = self._operator_str + "{0:.4g}".format(self._target)
    if self._fuzzyness > 0:
      result = result + "~{0:.4g}".format(self._fuzzyness)
    return result

  def Compare(self, value):
    """
    compares a number with the comparison string provided when constructing
    this object. The return value is:
      False: if the value is out of the valid range
      1: if the value is matched by the unequality/equality operation
      0..1: if the value is somewhere in the range of validity.
    """
    score = self._operator(self, value)
    if score < 0:
      return False
    elif score > 1:
      return 1
    else:
      return score

  def _CompareEq(self, value):
    if self._fuzzyness > 0:
      return (self._fuzzyness - math.fabs(self._target - value)) / \
           self._fuzzyness
    else:
      return value == self._target

  def _CompareGte(self, value):
    if self._fuzzyness > 0:
      return self._CompareGt(value)
    else:
      return value >= self._target

  def _CompareGt(self, value):
    if self._fuzzyness > 0:
      return float(value - self._target + self._fuzzyness) / self._fuzzyness
    else:
      return value > self._target

  def _CompareLte(self, value):
    if self._fuzzyness > 0:
      return self._CompareLt(value)
    else:
      return value <= self._target

  def _CompareLt(self, value):
    if self._fuzzyness > 0:
      return float(self._target - value + self._fuzzyness) / self._fuzzyness
    else:
      return value < self._target

  _operator_mapping = {
    "==": _CompareEq,
    "=" : _CompareEq,
    ">=": _CompareGte,
    ">" : _CompareGt,
    "<=": _CompareLte,
    "<" : _CompareLt,
  }
