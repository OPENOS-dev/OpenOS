# Copyright (c) 2012 The Chromium OS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

all:
clean:
tests:
	PYTHONPATH=src python src/tests/all_tests.py

install:
	install -D -m 0755 src/fuzzy_check.py  $(DESTDIR)/framework/fuzzy_check.py
	install -D -m 0755 src/main.py         $(DESTDIR)/framework/main.py
	install -D -m 0755 src/table.py        $(DESTDIR)/framework/table.py
	install -D -m 0755 src/validators.py   $(DESTDIR)/framework/validators.py
	install -D -m 0755 src/test_case.py    $(DESTDIR)/framework/test_case.py
	install -D -m 0755 src/test_factory.py $(DESTDIR)/framework/test_factory.py
	install -D -m 0755 src/test_runner.py  $(DESTDIR)/framework/test_runner.py
	install -D -m 0755 src/test_verifier.py \
	                   $(DESTDIR)/framework/test_verifier.py