# Copyright 2024 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
EAPI=7

inherit arc-build-constants udev

DESCRIPTION="Ebuild which pulls in any necessary ebuilds as dependencies
or portage actions."
HOMEPAGE=""
SRC_URI=""

LICENSE="BSD-Google"
SLOT="0"
KEYWORDS="-* arm64 arm"
S="${WORKDIR}"
IUSE="cheets"

#Add dependencies on other ebuilds from within this board overlay
DEPEND="
	chromeos-base/chromeos-scp-firmware-skywalker
"
RDEPEND="${DEPEND}"
BDEPEND=""

src_install() {
	# Override default CPUFreq governor
	insinto "/etc"
	doins "${FILESDIR}/cpufreq.conf"

	# udev rules for codecs
	insinto "/etc/init"
	doins "${FILESDIR}/udev-trigger-codec.conf"
	udev_dorules "${FILESDIR}/50-media.rules"

	# skywalker-specific upstart job to accommodate MCDI init delay
	doins "${FILESDIR}/mcdi-init-wait.conf"

	# Install cpuset adjustments.
	if use cheets; then
		arc-build-constants-configure

		insinto "${ARC_PREFIX:?}/vendor/etc/init"
		doins "${FILESDIR}/init.cpusets.rc"
	fi
}
