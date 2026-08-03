# Copyright 2016 The Chromium OS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

from fuzzy_check import FuzzyCheck
from validators import *

# originally generated gestures:
# FlingStop
#   ButtonDown(1)
#   ButtonUp(1)
#   FlingStop
#   Motion d=0.4468 x=0.4468 y=0 r=5.551e-17 s=26.71
#   FlingStop
#   FlingStop
#   FlingStop
#   FlingStop
#   FlingStop
#   FlingStop
#   FlingStop
#   FlingStop
#   FlingStop
#   FlingStop
#   FlingStop
#   FlingStop
#   FlingStop
#   FlingStop
#   Motion d=1085 x=739.1 y=785.8 r=44.53 s=2404
#   FlingStop
#   Motion d=73.02 x=69.79 y=20.53 r=2.876 s=726.7
#   FlingStop
#   ButtonDown(1)
#   ButtonUp(1)
#   FlingStop
#   Motion d=531.4 x=366.5 y=378.5 r=51.41 s=2645
#   FlingStop
#   Motion d=679.6 x=498.8 y=460.2 r=19.12 s=3121
#   FlingStop
#   Motion d=252.4 x=125.7 y=218.6 r=20.84 s=2516
#   FlingStop
#   Pinch dz=28.44 r=0.008984
#   FlingStop
#   Pinch dz=30.56 r=0.01178
#   FlingStop
#   Pinch dz=33.8 r=0.004329
#   FlingStop
#   Pinch dz=29.4 r=0.01254
#   FlingStop
#   Pinch dz=40.28 r=0.007549
#   FlingStop
#   FlingStop
#   Pinch dz=20.2 r=0.006354
#   FlingStop
#   Pinch dz=20 r=0.01703
#   FlingStop
#   Pinch dz=59.19 r=0.01019
#   FlingStop
#   Pinch dz=26.49 r=0.01084
#   FlingStop
#   Motion d=16.49 x=0 y=16.49 r=2.257 s=491.7
#   FlingStop
#   Scroll d=92.09 x=0 y=92.09 r=0.399 s=134.1
#   Fling d=103.4 x=0 y=103.4 r=0 s=3391
#   FlingStop
#   Scroll d=31.42 x=0 y=31.42 r=0.1693 s=98.76
#   Fling d=71.71 x=0 y=71.71 r=1.421e-14 s=2343
#   FlingStop
#   Scroll d=21.86 x=0 y=21.86 r=0.1379 s=130.6
#   Fling d=127.9 x=0 y=127.9 r=1.421e-14 s=4181
#   FlingStop
#   Scroll d=19.9 x=0 y=19.9 r=0.4459 s=170.9
#   Fling d=176.3 x=0 y=176.3 r=0 s=5749
#   FlingStop
#   Scroll d=30.13 x=0 y=30.13 r=0.9851 s=300
#   Fling d=336.5 x=0 y=336.5 r=5.684e-14 s=1.103e+04
#   FlingStop
#   Scroll d=41.16 x=0 y=41.16 r=2.635 s=492
#   Fling d=441.3 x=0 y=441.3 r=5.684e-14 s=1.447e+04
#   FlingStop
#   FlingStop
#   Motion d=0.8936 x=0 y=0.8936 r=5.551e-17 s=10.34
#   FlingStop
#   FlingStop
#   FlingStop
#   FlingStop
#   FlingStop
#   FlingStop

def Validate(raw, events, gestures):
  fuzzy = FuzzyCheck()
  fuzzy.expected = [
    MotionValidator(">= 800"),
    MotionValidator(">= 800"),
    MotionValidator(">= 700"),
    MotionValidator(">= 1100"),
    MotionValidator(">= 800"),
    MotionValidator(">= 1000"),
    MotionValidator(">= 1000"),
  ]
  fuzzy.unexpected = [
    FlingStopValidator("<10"),
  ]
  return fuzzy.Check(gestures)
