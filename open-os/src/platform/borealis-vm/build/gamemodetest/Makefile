# Copyright 2021 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

all: gamemodealloc gamemodeglalloc gamemodewin

gamemodealloc: gamemodealloc.c
	$(CC) -o $@ -g -O0 -Werror -Wall $<

gamemodeglalloc: gamemodeglalloc.cc
	$(CXX) $$(pkg-config --cflags glfw3) -o $@ \
		-g -Werror -Wall -Ithird_party \
		$^ \
		$$(pkg-config --static --libs epoxy) \
		$$(pkg-config --static --libs glfw3)

gamemodewin: gamemodewin.c
	$(CC) -o $@ -g -O0 -Werror -Wall -I/usr/include/SDL2 $< -lSDL2 -ludev

clean:
	$(RM) -f gamemodealloc gamemodeglalloc gamemodewin
