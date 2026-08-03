# Copyright (c) 2014 The Chromium OS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# This module provides the test runner which is responsible for discovering
# and executing tests.
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import logging
import multiprocessing
import traceback

from mtreplay import MTReplay
from mtlib import Log
from mtlib.gesture_log import GestureLog
from os import path
from test_case import TestCase

class TestRunner(object):
  """
  The TestRunner provides a simple interface to execute a set of test cases.
  All test results are returned in a result structure containing logs,
  validation results, errors, the generated gestures and the final score.
  """
  def __init__(self, tests_dir):
    self.tests_dir = tests_dir

  def RunTest(self, test_case, generate_original_values=False):
    """
    This method executes a single test. The results are provided in the
    following structure:

    {
      "logs": {
        "evdev": ""       # libevdev logs
        "gestures": "",   # gestures library logs
        "activity": "",   # generated activity log
        "validation": ""  # logs from the validation process
      },
      "error": "",        # error message if applicable.
      "events": "",       # string representation of all events generated
      "gestures": "",     # string representation of all gestures generated
      "result": "",       # "success" the test provides a score.
                          # "failure" the test executed, but validation failed.
                          # "error" the test could not be executed.
      "score": 0          # the final score of the test
    }
    """
    out = {
      "logs": {
        "evdev": "",
        "gestures": "",
        "activity": "",
        "validation": ""
      },
      "error": "",
      "events": "",
      "description": "",
      "disabled": False,
      "gestures": "",
      "result": "",
      "score": 0
    }

    dbg_log = logging.getLogger(test_case.name)

    if not test_case.IsComplete():
      out["result"] = "incomplete"
      out["disabled"] = True
      return out

    result = test_case.Check()
    if result is not True:
      out["error"] = result
      out["result"] = "error"
      return out

    out["description"] = test_case.description

    if test_case.disabled:
      out["result"] = "disabled"
      out["disabled"] = True
      return out

    try:
      dbg_log.info("Loading log: %s", test_case.log_file)
      events = Log(evdev=test_case.log_file)
      dbg_log.info("Starting replay...")
      replay = MTReplay()
      replay_results = replay.Replay(events, test_case.gesture_props,
                                     test_case.platform, dbg_log=dbg_log)
      log = replay_results.gestures

      out["logs"]["evdev"] = replay_results.evdev_log
      out["logs"]["gestures"] = replay_results.gestures_log
      out["logs"]["activity"] = replay_results.log.activity
      out["events"] = "\n".join(map(lambda e: str(e), log.events))
      out["gestures"] = "\n".join(map(lambda g: str(g), log.gestures))

      # look for errors in the gestures library output
      errors = []
      for line in replay_results.gestures_log.splitlines():
        if line.startswith("ERROR:"):
          errors.append(line)

      if errors:
        dbg_log.info("Found errors in gestures log")
        out["error"] = "\n".join(errors)
        out["result"] = "error"
        return out

      # validate the replay results
      dbg_log.info("Validating...")
      if generate_original_values:
        score, validation_log, original_values = \
            test_case.module.GenerateOriginalValues(
          log.raw, log.events, log.gestures)
        out['original_values'] = original_values
      elif test_case.original_values:
        score, validation_log = test_case.module.Validate(
          log.raw, log.events, log.gestures,
          original_values=test_case.original_values)
      else:
        score, validation_log = test_case.module.Validate(
          log.raw, log.events, log.gestures)

      out["logs"]["validation"] = validation_log
      out["result"] = "success" if score is not False else "failure"
      out["score"] = score
      dbg_log.info("Validation result: %s", out["result"])

    except:
      dbg_log.exception("Exception raised")
      out["error"] = traceback.format_exc()
      out["result"] = "error"

    return out

  def RunGDB(self, glob):
    test_case = self.DiscoverTestCases(glob)[0]
    replay = MTReplay()
    events = Log(evdev=test_case.log_file)
    replay_results = replay.Replay(events, test_case.gesture_props,
                                   test_case.platform, gdb=True)

  def RunAll(self, glob="all"):
    """
    This method runs a set of test cases. Test cases can be selected using
    a unix-style GLOB or "all" to execute all tests.
    The result is a dictionary mapping the test name to the results structure
    as described in RunTest.
    """
    results = {}
    for case in self.DiscoverTestCases(glob):
      results[case.name] = self.RunTest(case)
    return results

  def DiscoverTestCases(self, glob=None):
    return TestCase.DiscoverTestCases(self.tests_dir, glob)


def ParallelTestRunnerProcess(case, verbose=False):
  level = logging.INFO if verbose else logging.WARNING
  logging.basicConfig(level=level)

  logging.info("Starting process for: %s", case.name)
  runner = TestRunner(case.tests_path)
  return (case, runner.RunTest(case))


def VerboseParallelTestRunnerProcess(case):
  return ParallelTestRunnerProcess(case, verbose=True)


class ParallelTestRunner(TestRunner):
  def RunAll(self, glob="all", verbose=False):
    """
    Extends TestRunner.RunAll by multiprocessing. The test cases are distributed
    to as many processes as there are cores available.
    """
    pool = multiprocessing.Pool()
    cases = self.DiscoverTestCases(glob)
    logging.info("Starting multiprocessing pool")
    if logging.getLogger().isEnabledFor(logging.INFO):
      data = pool.map(VerboseParallelTestRunnerProcess, cases)
    else:
      data = pool.map(ParallelTestRunnerProcess, cases)
    logging.info("All tests executed")
    pool.terminate()
    results = {}
    for case, result in data:
      results[case.name] = result

    return results
