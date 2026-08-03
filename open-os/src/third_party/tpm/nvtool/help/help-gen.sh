#! /bin/sh

# Copyright (c) 2009,2010 The Chromium OS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

sed -e 's#\\#\\\\#g' -e 's#"#\\"#g' -e 's#^#"#g' -e 's#$#\\n"#g' help/help.txt > help/help.h
