# Copyright 2024 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

EAPI=7

CROS_WORKON_COMMIT="d2d95e8af89939f893b1443135497c1f5572aebc"
CROS_WORKON_TREE="776139a53bc86333de8672a51ed7879e75909ac9"
inherit appid arc-build-constants cros-unibuild cros-workon udev

# This ebuild only cares about its own FILESDIR and ebuild file, so it tracks
# the canonical empty project.
CROS_WORKON_PROJECT="chromiumos/infra/build/empty-project"
CROS_WORKON_LOCALNAME="platform/empty-project"

DESCRIPTION="Ebuild which pulls in any necessary ebuilds as dependencies
or portage actions."

LICENSE="BSD-Google"
SLOT="0"
KEYWORDS="-* arm64 arm"
IUSE=""

# Add dependencies on other ebuilds from within this board overlay
RDEPEND="
	chromeos-base/chromeos-config
	chromeos-base/chromeos-bsp-baseboard-skywalker
	chromeos-base/lte_power_control
	chromeos-base/touch_updater
"
DEPEND="${RDEPEND}"

src_install() {
	doappid "{DF097AF0-D652-4B6F-8D58-5B172C4E3539}" "CHROMEBOOK"

	# Install Proximity sensor rules
	udev_dorules "${FILESDIR}/99-cros-sx-proximity.rules"

	# Install audio config files
	unibuild_install_files audio-files

	insinto /etc/init
	doins "${FILESDIR}/upstart/set_lru_gen_min_ttl_ms.conf"
}
