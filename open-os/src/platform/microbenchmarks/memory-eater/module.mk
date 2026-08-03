# Copyright 2017 The Chromium OS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

include common.mk

CC_BINARY(memory-eater/memory-eater): memory-eater/memory-eater.o

clean: CLEAN(memory-eater/memory-eater)

all: CC_BINARY(memory-eater/memory-eater)
