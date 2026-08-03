# Copyright (c) 2014 The Chromium OS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

from fuzzy_check import FuzzyCheck
from validators import *

# originally generated gestures:
# FlingStop
#   Motion d=863.6 x=649.5 y=550.5 r=2.986 s=923.6
#   FlingStop
#   FlingStop
#   ButtonDown(1)
#   ButtonUp(1)
#   FlingStop
#   Motion d=539.3 x=213.9 y=473.1 r=1.879 s=1128
#   FlingStop
#   Motion d=238.3 x=63.14 y=227.6 r=3.385 s=1997
#   FlingStop
#   Motion d=725.8 x=719.1 y=91.93 r=10.01 s=3644
#   FlingStop
#   Motion d=325.3 x=66.04 y=318.2 r=17.32 s=4077
#   FlingStop
#   Motion d=403 x=164.6 y=366.2 r=4.717 s=1558
#   FlingStop
#   Motion d=305.8 x=294.6 y=74.57 r=1.595 s=529.7
#   FlingStop
#   Motion d=311.4 x=32.81 y=308.8 r=0.9472 s=894.4
#   FlingStop
#   Motion d=138.5 x=106.3 y=86.43 r=0.4178 s=496
#   FlingStop
#   ButtonDown(1)
#   ButtonUp(1)
#   FlingStop
#   Motion d=795.4 x=554 y=484.4 r=0.953 s=629.4
#   FlingStop
#   Motion d=1623 x=1316 y=795.6 r=7.009 s=2764
#   FlingStop
#   Motion d=177.8 x=174.5 y=19.31 r=2.113 s=661.5
#   FlingStop
#   Motion d=126.1 x=81.22 y=95.63 r=0.4865 s=487.2
#   FlingStop
#   Motion d=232.6 x=211.1 y=87.13 r=0.8799 s=458.3
#   Motion d=296.3 x=292.8 y=26.69 r=0.7937 s=504.3
#   FlingStop
#   Motion d=945.3 x=789.6 y=462.7 r=2.21 s=1284
#   Motion d=88.1 x=9.716 y=87.42 r=0.6588 s=354.5
#   Motion d=25.09 x=16.29 y=18.31 r=0.03725 s=27.41
#   Motion d=0.1441 x=0.1441 y=0 r=0 s=14.5
#   Motion d=0.7804 x=0.2883 y=0.5765 r=0.05347 s=25.84
#   FlingStop
#   Motion d=118.2 x=36.75 y=111.5 r=0.3182 s=224
#   FlingStop
#   Motion d=194 x=14.69 y=192.8 r=4.132 s=1499

def Validate(raw, events, gestures):
  for entry in raw['entries']:
    if entry['type'] == 'gesture':
      if entry['endTime'] > 8703.338468:
        return False, "Has activity during lift"
  return True, "Success"
