#!/usr/bin/awk -f

# Copyright 2024 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

BEGIN {}

{
    # Sample BtClient output showing device information:
    #
    # btclient:info: hci0 enabled = true
    # ...
    # btclient:info: Class: 9600
    # ...
    # btclient:info: Client exiting
    if ($2 == "Class:") {
        print $3
    }
}

END {}
