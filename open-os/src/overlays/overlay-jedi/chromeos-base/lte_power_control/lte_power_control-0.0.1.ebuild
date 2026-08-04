# Copyright 2025 The Chromium OS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

EAPI=7

DESCRIPTION="Init power sequence for Rolling Wireless LTE module"

LICENSE="BSD-Google"
SLOT="0"
KEYWORDS="-* arm64 arm"
IUSE=""
S="${WORKDIR}"

src_install() {
	dosbin "${FILESDIR}"/lte_power_control
	insinto /etc/init
	doins "${FILESDIR}"/lte_power_on.conf
	doins "${FILESDIR}"/lte_power_off.conf
}
