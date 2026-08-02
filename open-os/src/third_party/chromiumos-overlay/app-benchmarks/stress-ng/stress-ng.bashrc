# Copyright 2020 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# stress-ng uses -Werror for this single config check, which makes it think we
# don't have `ustat` available.
export MAKEOPTS+=" HAVE_USTAT=1"

# It also has internal conflicts in its cmdline definitions for `-DVERSION`
# (b/384064327). Demote those to warnings so `./configure` is more accurate.
export CPPFLAGS+=" -Wno-error=macro-redefined"
