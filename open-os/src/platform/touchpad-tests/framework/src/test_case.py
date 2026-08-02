# Copyright (c) 2012 The Chromium OS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# This module contains the TestCase class which gives access to the data files
# belonging to a test case and validation that the TestCase has all required
# information.
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

from mtlib.gesture_log import GestureLog
from os import path
import fnmatch
import imp
import json
import os
import traceback

class TestCase(object):
  """
  The TestCase class is constructed with test name and a tests directory.
  In order to automatically load properties from xorg config files, the
  xorg_conf_path argument has to be set to the path of the folder containing
  the chromeos xorg configuration files.

  Tests have multiple files associated with them and these file paths are
  determined using the convention over configuration paradigm:

  The tests directory contains one folder for each platform (lumpy, cr48, etc)
  as well as a common folder which is shared by all platforms. Each platform
  has to contain a platform.dat containing the platforms touchpad description
  (see replay tool and evemu). Additionally it can provide a platform.props file
  to pass properties to the gestures library.
  Each platform can organize test cases in subfolders.

  A test case is defined by a log file containing the event log and the name
  $(testname).log.
  The gestures from the log are validated using the python module in the same
  folder $(testname).py. If such a file cannot be found, the class will look for
  a file $(testname).py in same subdirectory of the common folder.

  Each test case can define parameters for the gestures library using a JSON
  file called $(testname).props. These properties override any properties set
  in platform.props.

  To make sure all those files are present and the test can be executed this
  class provides a method called Check which will return an error message
  in case there is an error.

  Tests are basically named after the path to the tests log file (including the
  platform name) but excluding the .log extension. So the log file
  $(tests_dir)/lumpy/categyoryA/testB.log has the name lumpy/categoryA/testB
  """
  _platform_dat = "platform.dat"
  _platform_props = "platform.props"
  _common_folder = "common"

  def __init__(self, tests_path, name):
    self.tests_path = tests_path
    self._ignore_properties = []

    # remove extension to get test name in case a filename was provided
    self.testname = path.splitext(name)[0]
    self.path = self.testname.split(os.sep)
    self.properties = None
    self.platform = None
    self.original_values = None
    self._module = None

  def IsComplete(self):
    return path.exists(self.log_file) and (self.module is not None)

  def Check(self):
    """
    Returns an error message describing the configuration error, or True if
    the test can be executed.
    """
    if not path.exists(self.log_file):
      return self.log_file + " does not exist"

    # catch all errors caused by loading the module
    try:
      if self.module is None:
        return "cannot find module file"
    except:
      return "unable to load module\n" + traceback.format_exc()

    if "Validate" not in self.module.__dict__:
      return self.module.__file__ + " does not contain a Validate() method"

    return True

  def __str__(self):
    return self.name

  def __repr__(self):
    return self.name

  @property
  def name(self):
    return "/".join(self.path)

  @property
  def platform_name(self):
    return self.path[0]

  @property
  def platform_dat_file(self):
    return self._PlatformFile(self._platform_dat)

  @property
  def log_file(self):
    return self._TestCaseFile(".log")

  @property
  def case_module_file(self):
    return self._TestCaseFile(".py")

  @property
  def common_module_file(self):
    return self._CommonFile(".py")

  @property
  def case_property_file(self):
    return self._TestCaseFile(".props")

  @property
  def device_property_file(self):
    return self._PlatformFile("platform.props")

  @property
  def common_property_file(self):
    return self._CommonFile(".props")

  @property
  def description(self):
    module = self.module
    return module.Validate.__doc__ if module.Validate.__doc__ else ""

  @property
  def user_description(self):
    module = self.module
    if hasattr(module, "UserInstructions"):
      return module.UserInstructions()
    return None

  @property
  def disabled(self):
    module = self.module
    return hasattr(module.Validate, "disabled") and module.Validate.disabled

  @property
  def disabled(self):
    module = self.module
    return hasattr(module.Validate, "disabled") and module.Validate.disabled

  @property
  def takes_original_values(self):
    return hasattr(self.module, "GenerateOriginalValues")

  @property
  def is_virtual(self):
    if self.module_file:
      module_name = path.basename(path.splitext(self.module_file)[0])
      log_name = path.basename(path.splitext(self.log_file)[0])
      return log_name != module_name
    return True

  @property
  def module_file(self):
    def FindHierachicalModuleFile(file):
      if path.exists(file + ".py"):
        return file + ".py"
      elif '_' in file:
        return FindHierachicalModuleFile(file[:file.rfind("_")])
      return None

    module_file = FindHierachicalModuleFile(self._TestCaseFile(""))
    if module_file:
      return module_file
    module_file = FindHierachicalModuleFile(self._CommonFile(""))
    if module_file:
      return module_file
    return None

  @property
  def module(self):
    module_file = self.module_file
    if not module_file:
      return None
    name = os.path.splitext(os.path.basename(module_file))[0]
    return imp.load_source(name, module_file)

  @property
  def gesture_props(self):
    if self.properties is None:
      self._LoadProperties()
    return self.properties["gestures"]

  @property
  def validator_props(self):
    if self.properties is None:
      self._LoadProperties()
    return self.properties["validator"]

  @property
  def ignore_properties(self):
    if self.properties is None:
      self._LoadProperties()
    return self._ignore_properties

  @property
  def device_class(self):
    if self.properties is None:
      self._LoadProperties()
    return self.properties.get("device_class")

  @property
  def instruct_robot(self):
    if "InstructRobot" in self.module.__dict__:
      return self.module.InstructRobot
    return None

  def ReloadProperties(self):
    self.properties = None
    self._LoadProperties()

  def _LoadProperties(self):
    self.properties = {
      # The "Event Logging Enable" property is required for the gestures library
      # to produce the event logs used by these tests.
      "gestures": { "Event Logging Enable" : True },
      "validator": {},
    }
    self._LoadPropertiesFile(self._PlatformFile("platform.props"))
    self._LoadPropertiesFile(self._CommonFile(".props"))
    self._LoadPropertiesFile(self._TestCaseFile(".props"))
    self._ApplyIgnoreProperties()

  def _ApplyIgnoreProperties(self):
    for key in self._ignore_properties:
      if key in self.properties["gestures"]:
        del self.properties["gestures"][key]

  def _LoadPropertiesFile(self, file):
    if not path.exists(file):
      return
    data = json.load(open(file, "r"))
    if "gestures" in data:
      self.properties["gestures"].update(data["gestures"])
    if "validator" in data:
      self.properties["validator"].update(data["validator"])
    if "ignore" in data:
      self._ignore_properties.extend(data["ignore"])
    if "device_class" in data:
      self.properties["device_class"] = data["device_class"]
    if "platform" in data:
      self.platform = data["platform"]
    if "original_values" in data:
      self.original_values = data["original_values"]

  def _TestCaseFile(self, extension):
    return os.path.join(self.tests_path, *self.path) + extension

  def _CommonFile(self, extension):
    return os.path.join(self.tests_path, self._common_folder, *self.path[1:]) \
           + extension

  def _PlatformFile(self, filename):
    return os.path.join(self.tests_path, self.path[0], filename)

  @staticmethod
  def DiscoverTestCases(tests_dir, glob=None):
    """ Returns a list of test cases that matching the glob. """
    def ListDirRecursive(rootdir, sub_path=""):
      result = []
      target_path = path.join(rootdir, sub_path)
      if not path.exists(target_path):
        return []
      for file in os.listdir(target_path):
        if file[0] == ".":
          continue
        if path.isdir(path.join(rootdir, sub_path, file)):
          result.extend(ListDirRecursive(rootdir, path.join(sub_path, file)))
        else:
          result.append(path.join(sub_path, file))
      return result

    def ListCases(tests_dir, platform):
      files = ListDirRecursive(path.join(tests_dir, platform), "")
      py_files = fnmatch.filter(files, "*.py")
      log_files = fnmatch.filter(files, "*.log")
      test_names = map(lambda p: path.splitext(p)[0], py_files + log_files)
      return set(test_names)

    common_validators = ListCases(tests_dir, "common")
    all_cases = []
    for platform in TestCase.DiscoverPlatforms(tests_dir):
      validators = ListCases(tests_dir, platform)
      device_class = TestCase(tests_dir,
              path.join(platform, "dummy")).device_class
      if device_class is None or device_class == "touchpad":
        validators.update(common_validators)

      test_cases = map(lambda f: TestCase(tests_dir, path.join(platform, f)),
                       validators)
      all_cases.extend(test_cases)

    all_cases = filter(lambda c: c.module, all_cases)

    if glob is not None and glob != "all":
      all_cases = filter(lambda c: fnmatch.fnmatch(c.name, glob) or
                                   c.name.startswith(glob), all_cases)

    return all_cases

  @staticmethod
  def DiscoverPlatforms(tests_dir):
    def is_platform(platform):
      return platform[0] != "." and path.isdir(path.join(tests_dir, platform))
    return filter(is_platform, os.listdir(tests_dir))
