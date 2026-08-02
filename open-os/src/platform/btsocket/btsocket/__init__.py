# Copyright (c) 2013 The Chromium OS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# That btsocket is actually a package of mixed C and Python code is an
# implementation detail. Import the exported pieces such that btsocket.*
# presents a single-level API.
from .constants import *
from .btsocket import BluetoothSocket as socket
from ._btsocket import error, timeout
