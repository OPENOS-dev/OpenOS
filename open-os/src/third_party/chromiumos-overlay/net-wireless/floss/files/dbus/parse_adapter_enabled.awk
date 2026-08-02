#!/usr/bin/awk -f

# Copyright 2024 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

BEGIN {}

{
    # Sample D-Bus output:
    #
    # method return time=1710376401.456988 sender=:1.24... [truncated]
    #   boolean true
    if ($1 == "boolean") {
        print $2
        exit
    }
}

END {}
