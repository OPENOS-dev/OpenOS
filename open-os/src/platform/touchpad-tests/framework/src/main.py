# Copyright 2012 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# This module is the main module for the console interface. It takes care
# of parsing the command line arguments and formating the output
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

from optparse import OptionParser
from subprocess import Popen, PIPE, STDOUT
from tempfile import NamedTemporaryFile
import json
import logging
import math
import multiprocessing
import os
import sys

from mtedit import MTEdit
from mtlib import Log, PlatformDatabase

from table import Table
from test_case import TestCase
from test_factory import TestFactory
from test_robot import RobotTestGenerator
from test_collector import TestCollector
from test_runner import ParallelTestRunner as TestRunner


_help_text = """\
Multitouch Regression Test Suite:
---------------------------------

$ %prog [all|glob]
Executes tests. Either all of them or selected tests by providing a glob match
In order to test for regressions use:
$ %prog all --out filename
make a change
$ %prog all --ref filename
Which will display the changes in test results compared to before the change

$ %prog testname -v %info%
Run test and display information. %info% can be:
- a or activity: to view the touchpad activity in mtedit
- g or gestures: to view the generated gestures
- al or activity-log: to view the generated activity log
- gl or gestures-log: to view the generated gestures log
- el or evdev-log: to view the generated evdev log

$ %prog test_name -c %source%
Create a new test case from %source%. Source can be:
- A feedback URL
- A device IP
- The path to an activity log file
When using a file name test.log, %prog will look at test.log.evdev
for the evdev log file. You can optionally supply -e to override this path.
%prog will display an URL where you can trim the log file, the trimmed
log file will then be used to create the new test case. Specify --no-edit in
case you do not want to use the original files without trimming.

$ %prog test_name --gdb
Run the test using gdb for debugging the gestures library

General Info:
-------------
testname arguments:
Tests are always names as [platform]/[name of test case]. You can find the tests
available in the tests folder.
For example: lumpy/scroll_test

Tests Folder:
The tests folder contains a folder for each platform and all the test cases.
Each test case is made up of 3 files:
[testcase].py which contains the validation script
[testcase].log which contains the input_event log
[testcase].props which contains user_defined properties passed to gestures lib.

Platform folders:
To add a new platform you need to add a new folder to the Tests folder, and
generate a platform.dat file. This can be done using the evemu-describe tool
on the target platform:

$ gmerge utouch-evemu
$ evemu-describe /path/to/device > platform.dat
"""


def Compile():
  if "SRC_DIR" not in os.environ:
    print("Requires SRC_DIR env-var. Re-run $ sudo make setup-in-place")
    sys.exit(1)

  dir = os.environ["SRC_DIR"]
  print("Recompiling gestures/libevdev/replay...")
  print("SRC_DIR is %s" % dir)
  process = Popen(["make", "-j", str(multiprocessing.cpu_count()),
                  "in-place"], cwd=dir, stdout=PIPE, stderr=STDOUT)
  ret = process.wait()
  if ret != 0:
    print(process.stdout.read().decode(errors='replace'))
    sys.exit(1)


def RunAndCompare(glob, ref_file=None, autotest=False):
  if not autotest:
    Compile()
  print("Running tests...")
  runner = TestRunner(os.environ["TESTS_DIR"])
  results = runner.RunAll(glob)

  # load reference
  ref = {}
  deltas = {}
  if ref_file:
    ref = json.load(open(ref_file))
    for name, res in results.items():
      if name in ref:
        deltas[name] = res["score"] - ref[name]["score"]

  return (results, ref, deltas)

def MakeResultsTable(title, names_to_include, results, ref, deltas):
  """Returns a table of test results."""
  def ResultStr(value):
    if value["result"] == "success":
      msg = "success" if value["score"] >= 0.5 else "bad"
      return "%s (%.4f)" % (msg, value["score"])
    else:
      return value["result"]

  def ColoredDelta(delta):
    if math.fabs(delta) < 1e-10:
      return "\x1b[0m%.4f\x1b[0m" % delta    # default color
    elif delta < 0:
      return "\x1b[31m%+.4f\x1b[0m" % delta  # red
    else:
      return "\x1b[32m%+.4f\x1b[0m" % delta  # green

  table = Table()
  table.title = title
  table.header("Test", "reference score", "new score", "delta")
  for name in sorted(names_to_include):
    ref_score = ResultStr(ref[name]) if name in ref else "(new)"
    delta_str = ColoredDelta(deltas[name]) if name in deltas else ""
    table.row(name, ref_score, ResultStr(results[name]), delta_str)

  return table

def ManualRun(glob, out_file=None, ref_file=None, autotest=False,
              compact_results=False):
  """
  Runs the specified tests, printing a table of results as well as logs for
  failures (if ref_file isn't specified) or regressions (if it is).
  """
  results, ref, deltas = RunAndCompare(glob, ref_file, autotest)

  # print reports
  sorted_results_items = sorted(results.items())
  for case_name, value in sorted_results_items:
    if case_name in ref:
      should_show_report = math.fabs(deltas[case_name]) >= 1e-10
    else:
      should_show_report = value["result"] != "success"
    if not should_show_report and len(results) > 1:
      continue

    print("### Validation report for", case_name)
    print(value["description"])
    if value["disabled"]:
      print("DISABLED")
    else:
      print(value["logs"]["validation"])
      print(value["error"])

  print(MakeResultsTable("Test Results", results.keys(), results, ref, deltas))

  if out_file:
    if compact_results:
      for test_name in results:
        r = results[test_name]
        del r["logs"], r["events"], r["gestures"]

    json.dump(results, open(out_file, "w"), indent=2, sort_keys=True)
    print("results stored in:", out_file)

  regression = any(name in ref and deltas[name] < 0 for name in results.keys())
  if regression:
    print("\x1b[91mThere are regressions present in this test run!\x1b[0m")
    sys.exit(1)


_IMPROVEMENT_MESSAGE = """
Some tests have been fixed or had their scores improved! Please update
tools/touchtests-report.json by running the following in the chroot:

    $ touchtests --compact-results \\
        --out ~/trunk/src/platform/gestures/tools/touchtests-report.json

Then add commit the JSON file changes to your CL.
"""

def PresubmitRun(ref_file, autotest=False):
  """
  Runs a gestures library presubmit, meaning it runs all tests, and exits with
  an error if any have regressed or been fixed (so that the user can update the
  reference file).
  """
  results, ref, deltas = RunAndCompare("all", ref_file, autotest)

  new_tests = results.keys() - ref.keys()
  failing_new_tests = set(filter(lambda n: results[n]["result"] != "success",
                                 new_tests))
  regressions = set(filter(lambda name: deltas[name] <= -1e-5, deltas.keys()))
  problems = regressions | failing_new_tests
  if len(problems) > 0:
    print(MakeResultsTable("Regressions or failures", problems, results, ref,
                           deltas))

  if len(problems) > 0:
    print("\x1b[91mPlease fix these failures or regressions, then re-run the "
          "presubmit.\x1b[0m")
    sys.exit(1)

  fixed_tests = set(filter(lambda name: deltas[name] >= 1e-5, deltas.keys()))
  if len(fixed_tests) > 0:
    print(MakeResultsTable("Improvements", fixed_tests, results, ref, deltas))
    print(_IMPROVEMENT_MESSAGE)
    sys.exit(1)


def Get(test_name, what, file=None):
  Compile()
  if file:
    data = json.load(open(file))
    results = data[test_name]
  else:
    runner = TestRunner(os.environ["TESTS_DIR"])
    data = runner.RunAll(test_name)
    results = data[test_name]

  if what == "gestures-log":
    print(results["logs"]["gestures"])
  elif what == "evdev-log":
    print(results["logs"]["evdev"])
  elif what == "activity-log":
    print(results["logs"]["activity"])
  elif what == "gestures":
    print(results["gestures"])
  elif what == "events":
    print(results["events"])
  elif what == "activity":
    log = Log(activity=results["logs"]["activity"])
    editor = MTEdit()
    editor.View(log)

def GDB(test_name):
  Compile()
  runner = TestRunner(os.environ["TESTS_DIR"])
  data = runner.RunGDB(test_name)

def Add(testname, log, gdb):
  """
  Adds a new test case.
  """
  # determine test name from activity_log name
  factory = TestFactory(os.environ["TESTS_DIR"])
  case = factory.CreateTest(testname, log, gdb)
  if case:
    print("Test \"" + case.name + "\" created")

def AddPlatform(ip):
  name = PlatformDatabase.RegisterPlatformFromDevice(ip)
  if not name:
    return
  dirname = os.path.join(os.environ["TESTS_DIR"], name)
  propsfile = os.path.join(dirname, "platform.props")
  if not os.path.exists(dirname):
    os.mkdir(dirname)
  open(propsfile, "w").write("{\"platform\": \"%s\"}" % name)
  print(" ", propsfile)

def Main():
  """
  Main entry point for the console interface
  """

  # setup paths from environment variables
  if "TESTS_DIR" not in os.environ:
    print("Require TESTS_DIR environment variable")
    sys.exit(1)

  TestCase.tests_path = os.environ["TESTS_DIR"]

  parser = OptionParser(usage=_help_text)
  parser.add_option("-c", "--create",
                    dest="create", default=None,
                    help="create new test case from URL/IP or log file")
  parser.add_option("-p", "--platform",
                    dest="platform", default=None,
                    help="specify platform when using --create")
  parser.add_option("-e", "--evdev",
                    dest="evdev", default=None,
                    help="path to evdev log for creating a new test")
  parser.add_option("-v", "--view",
                    dest="view", default=None,
                    help="view generated gestures(g), activity in mtedit(a) " +
                         "gestures-log(gl), evdev-log(el) or activity-log(al)")
  parser.add_option("-r", "--ref",
                    dest="ref", default=None,
                    help="reference test results for detecting regressions")
  parser.add_option("-o", "--out",
                    dest="out", default=None,
                    help="output test results to file.")
  parser.add_option("--compact-results",
                    dest="compact_results", action="store_true", default=False,
                    help="exclude logs from the test result file when using "
                         "--out, making it much smaller")
  parser.add_option("-n", "--new",
                    dest="new", action="store_true", default=False,
                    help="Create new device logs before downloading. " +
                         "[Default: False]")
  parser.add_option("--no-edit",
                    dest="noedit", action="store_true", default=False,
                    help="Skip editing when creating tests. Add original log " +
                         "[Default: False]")
  parser.add_option("--autotest",
                    dest="autotest", action="store_true", default=False,
                    help="Run in autotest mode. Skips recompilation.")
  parser.add_option("--gdb",
                    dest="gdb", action="store_true", default=False,
                    help="Run the test case in GDB"),
  parser.add_option("--verbose",
                    dest="verbose", action="store_true", default=False,
                    help="Verbose debug output"),
  parser.add_option("--robot",
                    dest="robot", default=None,
                    help="Instruct robot to generate test cases")
  parser.add_option("--collect_from",
                    dest="collect_ip", default=None,
                    help="Interactively collect tests at given device IP");
  parser.add_option(
    "--overwrite",
    dest="overwrite", action="store_true", default=False,
    help="(use with --robot or --collect_from) Overwrite existing tests")
  parser.add_option("--no-calib",
                    dest="nocalib", action="store_true", default=False,
                    help="(use with --robot) Skip calibration step.")
  parser.add_option("--manual-fingertips",
                    dest="manual_fingertips", action="store_true",
                    default=False,
                    help="(use with --robot) Use fingertips that are present.")
  parser.add_option("--slow",
                    dest="slow", action="store_true", default=False,
                    help="(use with --robot) Force slow movement.")
  parser.add_option("--add-platform",
                    dest="add_platform", default=None,
                    help="add platform from IP address of remote device.")
  parser.add_option("--presubmit",
                    dest="presubmit", action="store_true", default=False,
                    help="run tests as part of a Gestures library presubmit")

  (options, args) = parser.parse_args()
  options.download = False  # For compatibility with mtedit
  options.screenshot = False  # For compatibility with mtedit

  if options.add_platform:
    AddPlatform(options.add_platform)
    return

  if len(args) == 0:
    test_name = "all"
  elif len(args) == 1:
    test_name = args[0]
  else:
    parser.print_help()
    sys.exit(1)

  level = logging.INFO if options.verbose else logging.WARNING
  logging.basicConfig(level=level)

  if options.create:
    # obtain trimmed log data
    original_log = Log(options.create, options)
    if options.noedit:
      log = original_log
    else:
      editor = MTEdit()
      platform = options.platform or test_name.split(os.sep)[0]
      log = editor.Edit(original_log, force_platform=platform)

    # pass to touchtests
    Add(test_name, log, options.gdb)

  elif options.view:
    view = options.view
    if view == "g":
      view = "gestures"
    elif view == "gl":
      view = "gestures-log"
    elif view == "el":
      view = "evdev-log"
    elif view == "al":
      view = "activity-log"
    elif view == "a":
      view = "activity"
    Get(test_name, view)
  elif options.gdb:
    GDB(test_name)
  elif options.collect_ip:
    generator = TestCollector(options.collect_ip, os.environ["TESTS_DIR"])
    generator.GenerateAll(test_name, options.overwrite)
  elif options.robot:
    generator = RobotTestGenerator(options.robot, not options.nocalib,
        options.slow, options.manual_fingertips, os.environ["TESTS_DIR"])
    generator.GenerateAll(test_name, options.overwrite)
  elif options.presubmit:
    if options.ref is None:
      print("Error: --ref must be specified with --presubmit.")
      sys.exit(1)

    PresubmitRun(options.ref, options.autotest)
  else:
    ManualRun(test_name, options.out, options.ref, options.autotest,
              options.compact_results)


if __name__ == "__main__":
  Main()
