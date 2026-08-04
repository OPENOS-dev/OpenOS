# Copyright 2022 The ChromiumOS Authors
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
IUSE="cheets geralt-kernelnext"

# Add dependencies on other ebuilds from within this board overlay
RDEPEND="
	chromeos-base/chromeos-config
	chromeos-base/chromeos-bsp-baseboard-geralt
	chromeos-base/sof-binary
	chromeos-base/sof-topology
	chromeos-base/touch_updater
	media-sound/sound_card_init
"
DEPEND="${RDEPEND}"

src_install() {
	if use geralt-kernelnext; then
		doappid "{F5511B74-7685-47D0-BFD9-6BE494213F0A}" "CHROMEBOOK"
	else
		doappid "{F2F02F3F-E04D-4867-BA41-8E90F8A34A87}" "CHROMEBOOK"
	fi

	# Install audio config files
	unibuild_install_files audio-files

	# Install cpuset adjustments.
	if use cheets; then
		arc-build-constants-configure

		insinto "${ARC_PREFIX:?}/vendor/etc/init"
		doins "${FILESDIR}/init.cpusets.rc"
	fi

	insinto /etc/init
	doins "${FILESDIR}/upstart/set_lru_gen_min_ttl_ms.conf"

	# Install hammerd udev rules and override for chromeos-base/hammerd.
	udev_dorules "${FILESDIR}/99-hammerd.rules"
}
