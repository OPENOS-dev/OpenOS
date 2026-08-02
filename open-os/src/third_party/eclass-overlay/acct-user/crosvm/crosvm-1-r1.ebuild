# Copyright 2024 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

EAPI=7

inherit acct-user

DESCRIPTION="CrOS virtual machine monitor"

# NB: These settings are ignored in CrOS for now.
# See the files in profiles/base/accounts/ instead.

ACCT_USER_ID=299
ACCT_USER_GROUPS=( shadercached pluginvm video virtaccess daemon-store wayland arc-camera crosvm traced-producer cups-proxy tun )
