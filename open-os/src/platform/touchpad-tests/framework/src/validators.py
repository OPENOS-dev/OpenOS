# Copyright 2012 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# This module contains the Validators used by the FuzzyCheck class.
# The validators mak use of the FuzzyComparator class that allows validations
# to be made in a fuzzy way that leads to a score instead of a true/false
# decision.
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import math
import re

from fuzzy_check import FuzzyComparator

def Disabled(fn):
  """
  Function decorator to disable a test case.

    @Disabled
    def Validate():
      [...]
  """
  fn.disabled = True
  return fn

def FingerSize(*args):
  """
  Function decorator to define finger sizes on the robot instruction method.

    @FingerSize(1)
    def InstructRobot(robot):
      [...]
  """
  def Wrapper(fn):
    fn.finger_size = args
    return fn
  return Wrapper

def CustomFinger(fn):
  """
  Function decorator to enforce custom finger setup for an robot instruction
  method. The robot will start with no finger tips attached and this method
  has to take care of setting up finger tips as well as cleaning them up again.

    @CustomFinger
    def InstructRobot(robot):
      [...]
  """
  fn.finger_size = None
  return fn

class AbstractValidator(object):
  """
  Base class provides the possibility to Validate start/end times of
  gestures.
  """
  def __init__(self, start=None, end=None):
    if start:
      self._start_check = FuzzyComparator(start)
    else:
      self._start_check = None
    if end:
      self._end_check = FuzzyComparator(end)
    else:
      self._end_check = None

    self._start = 0
    self._end = 0

  def Accept(self, event):
    """
    This method is used to determine if this validator is able to validate a
    certain event. If it cannot validate the event it will be handled by
    the next validator or is considered as an unexpected event.
    """
    return False

  def Validate(self, event):
    """
    While this validator is active, this method is called for each gesture that
    it accepts (i.e. Accept(event) returns True).
    """
    pass

  def TypeStr(self):
    """
    Returns a shortened string representation used for presentation. It should
    not contain any validation information but only show which kind of
    gestures are accepted.
    """
    pass

  def _ValidateTimestamps(self, event):
    if self._start == 0:
      self._start = float(event.start)
    self._end = float(event.end)

  def _ScoreTimestamps(self):
    score = 1
    if self._start_check:
      score = score * self._start_check.Compare(self._start)
    if self._end_check:
      score = score * self._end_check.Compare(self._end)
    return score


class AbstractCountValidator(AbstractValidator):
  """
  Base class for buttons. Allows to Validate if a certain number of
  button events are present.
  """

  def __init__(self, count_match="== 1 ~ 1", start=None, end=None):
    AbstractValidator.__init__(self, start, end)
    self._count_match = FuzzyComparator(count_match)
    self._count = 0
    self.score = self._count_match.Compare(self._count)

  def Accept(self, event):
    return event.type == self.__class__.event_type

  def Validate(self, event):
    self._count = self._count + 1
    self.score = self._count_match.Compare(self._count)
    self._ValidateTimestamps(event)
    self.score = self.score * self._ScoreTimestamps()

  def TypeString(self):
    return self.__class__.event_type

  def __str__(self):
    format_str = "{0} c:{1}{2}"
    return format_str.format(self.TypeString(), self._count, self._count_match)


class FlingStopValidator(AbstractCountValidator):
  event_type = "FlingStop"

class SwipeLiftValidator(AbstractCountValidator):
  event_type = "SwipeLift"

class FourFingerSwipeLiftValidator(AbstractCountValidator):
  event_type = "FourFingerSwipeLift"


class AbstractButtonValidator(AbstractCountValidator):
  """
  Base class for buttons. Allows to Validate if a certain number of
  button events are present.
  """

  def __init__(self, button, count_match="== 1 ~ 1", start=None, end=None):
    AbstractCountValidator.__init__(self, count_match, start, end)
    self._button = button

  def Accept(self, event):
    return event.type == self.__class__.event_type and \
         event.button == self._button

  def TypeString(self):
    return self.__class__.event_type + "(" + str(self._button) + ")"


class ButtonDownValidator(AbstractButtonValidator):
  event_type = "ButtonDown"


class ButtonUpValidator(AbstractButtonValidator):
  event_type = "ButtonUp"


class AbstractAxisValidator(AbstractValidator):
  """
  Allows to Validate movement on a 2 dimensional axis (scroll, cursor movement,
  or fling. x and y movement can be validateValidated separately, as well as the
  overall distance traveled.
  Next to the overall movement, the Roughness measure of the movement can also
  be validateValidated.
  """
  def __init__(self, d=None, x=None, y=None, z=None, start=None, end=None,
               roughness=None, speed=None, merge=False, integralPixels=False):
    AbstractValidator.__init__(self, start, end)
    if d:
      self._distance_check = FuzzyComparator(d)
    else:
      self._distance_check = None
    if x:
      self._x_check = FuzzyComparator(x)
    else:
      self._x_check = None
    if y:
      self._y_check = FuzzyComparator(y)
    else:
      self._y_check = None
    if z:
      self._z_check = FuzzyComparator(z)
    else:
      self._z_check = None

    if speed:
      self._speed_check = FuzzyComparator(speed)
    else:
      self._speed_check = None

    if roughness:
      self._roughness_check = FuzzyComparator(roughness)
    else:
      self._roughness_check = None

    self._distance = 0
    self._x = 0
    self._y = 0
    self._z = 0
    self._distance_remainder = 0.0
    self._roughnesses = []
    self._speeds = []
    self._UpdateScore()
    self._merge = merge
    self._integralPixels = integralPixels

  def _Speed(self):
    if self._speeds:
      return sum(self._speeds) / len(self._speeds)
    return 0

  def Accept(self, event):
    return (event.type == self.event_type and
            (self._merge or
             not self._distance_check or
             self._distance_check.Compare(event.distance)) and
            ((float(event.start) - self._end) < 0.1 or
             self._end <= 0.0 or
             self._merge))

  def _UpdateScore(self):
    self.score = 1
    if self._distance_check:
      self.score = self.score * self._distance_check.Compare(self._distance)
    if self._x_check:
      self.score = self.score * self._x_check.Compare(self._x)
    if self._y_check:
      self.score = self.score * self._y_check.Compare(self._y)
    if self._z_check:
      self.score = self.score * self._z_check.Compare(self._z)
    if self._roughness_check:
      roughness = sum(self._roughnesses)
      self.score = self.score * self._roughness_check.Compare(roughness)
    if self._speed_check:
      self.score = self.score * self._speed_check.Compare(self._Speed())
    self.score = self.score * self._ScoreTimestamps()
    if self.score == 0:
      self.score = False

  def _AddDistance(self, event):
    if self._integralPixels:
      distance = self._distance + event.distance + self._distance_remainder
      self._distance = int(distance)
      self._distance_remainder = distance - self._distance
    else:
      self._distance = self._distance + event.distance

  def Validate(self, event):
    self._roughnesses.append(event.Roughness())
    self._speeds.append(event.Speed())
    self._AddDistance(event)
    self._x = self._x + event.dx
    self._y = self._y + event.dy
    self._z = self._z + event.dz
    self._ValidateTimestamps(event)
    self._UpdateScore()

  def TypeString(self):
    return self.event_type

  def __str__(self):
    valuators = []
    if self._distance_check:
      valuators.append("d:{0:.4g}".format(self._distance) +
                       str(self._distance_check))
    if self._x_check:
      valuators.append("x:{0:.4g}".format(self._x) + str(self._x_check))
    if self._y_check:
      valuators.append("y:{0:.4g}".format(self._y) + str(self._y_check))
    if self._z_check:
      valuators.append("z:{0:.4g}".format(self._z) + str(self._z_check))
    if self._roughness_check:
      valuators.append("r:{0:.4g}".format(sum(self._roughnesses)) +
                       str(self._roughness_check))
    if self._speed_check:
      valuators.append("s:{0:.4g}".format(self._Speed()) +
                       str(self._speed_check))
    if self._start_check:
      valuators.append("start:" + str(self._start) + str(self._start_check))
    if self._end_check:
      valuators.append("end:" + str(self._end) + str(self._end_check))
    return self.event_type + " " + (" ".join(valuators))


class FlingValidator(AbstractAxisValidator):
  event_type = "Fling"


class ScrollValidator(AbstractAxisValidator):
  event_type = "Scroll"


class PinchValidator(AbstractAxisValidator):
  event_type = "Pinch"


class SwipeValidator(AbstractAxisValidator):
  event_type = "Swipe"


class FourFingerSwipeValidator(AbstractAxisValidator):
  event_type = "FourFingerSwipe"


class MotionValidator(AbstractAxisValidator):
  event_type = "Motion"


class AnythingButValidator(object):
  """
  Allows to 'wait' for a certain gesture to be performed. It will accept all
  events exept for the events accepted by the validators passed as arguments.
  The score of this validator is always 1.
  """
  def __init__(self, *but):
    self._but = but
    self.score = 1

  def Accept(self, event):
    for validator in self._but:
      if validator.Accept(event):
        return False
    return True

  def Validate(self, event):
    pass

  def __str__(self):
    types = map(lambda x: str(x.TypeString()), self._but)
    return "not(" + " or ".join(types) + ")"
