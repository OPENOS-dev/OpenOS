#!/usr/bin/env python3
# Copyright (c) 2013 The Chromium OS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from setuptools import setup, Extension

ext1 = Extension('btsocket._btsocket', sources=['src/btsocket.c'])

setup(name='btsocket',
      version='1.0',
      description='Module for manipulating raw Bluetooth Sockets',
      packages=['btsocket'],
      ext_modules=[ext1])
