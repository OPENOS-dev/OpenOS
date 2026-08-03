# Copyright 2022 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

all: setup_bitmaps

setup_bitmaps: setup_bitmaps.c
	$(CC) $< -o $@ -Os -lext2fs

clean:
	$(RM) -f setup_bitmaps