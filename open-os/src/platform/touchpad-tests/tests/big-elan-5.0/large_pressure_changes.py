# Copyright (c) 2014 The Chromium OS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

from validators import *

# originally generated gestures:
# FlingStop
#   Motion d=489.1 x=105.5 y=474.7 r=6.31 s=1014

def Validate(raw, events, gestures):
  # To validate that not too many events are being suppressed, check that
  # most of the hwstates with fingers generated a move event.
  hwstates = [e for e in raw['entries']
              if e['type'] == 'hardwareState' and e['fingers']]
  move_events = [e for e in events if e.type == 'Motion']

  fraction_converted = float(len(move_events)) / float(len(hwstates))

  log = 'With %d hwstates with finger positions, \
         %d movement events were generated.' % (len(hwstates), len(move_events))

  return float(fraction_converted >= 0.75), log
