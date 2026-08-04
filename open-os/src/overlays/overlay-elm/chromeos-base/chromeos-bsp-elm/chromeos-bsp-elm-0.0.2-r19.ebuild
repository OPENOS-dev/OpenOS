# Copyright 2016 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

# This ebuild only cares about its own FILESDIR and ebuild file, so it tracks
# the canonical empty project.
CROS_WORKON_COMMIT="d2d95e8af89939f893b1443135497c1f5572aebc"
CROS_WORKON_TREE="776139a53bc86333de8672a51ed7879e75909ac9"
CROS_WORKON_PROJECT="chromiumos/infra/build/empty-project"
CROS_WORKON_LOCALNAME="platform/empty-project"

inherit appid cros-audio-configs udev cros-workon


DESCRIPTION="Ebuild which pulls in any necessary ebuilds as dependencies
or portage actions."

LICENSE="BSD-Google"
KEYWORDS="-* arm64 arm"
IUSE="elm-arc64 elm-cheets elm-connectivitynext elm64 kernel-5_15"

# Add dependencies on other ebuilds from within this board overlay
DEPEND=""
RDEPEND="${DEPEND}
	!<chromeos-base/gestures-conf-0.0.2
	chromeos-base/chromeos-bsp-baseboard-oak:=
	sys-apps/ethtool
"

src_install() {
	insinto "/etc/gesture"
	doins "${FILESDIR}"/gesture/*

	if use elm-arc64; then
		doappid "{AB7C8669-9930-4EE3-BC66-46C7F27CC8FA}" "CHROMEBOOK"
	elif use elm-cheets; then
		doappid "{3DFF3394-F97E-4971-83C6-2C5C06A9953D}" "CHROMEBOOK"
	elif use elm-connectivitynext; then
		doappid "{22EB1075-F58B-4881-BC88-E5FDC17BE66D}" "CHROMEBOOK"
	elif use elm64; then
		doappid "{8F4B3950-F029-40D9-AF7A-2D58E226EB66}" "CHROMEBOOK"
	else
		doappid "{5BF597B2-ADE3-52C9-1DDA-95719C914AFF}" "CHROMEBOOK"
	fi

	# Install audio config files.
	local audio_config_dir="${FILESDIR}/audio-config"
	if use kernel-5_15; then
		audio_config_dir="${FILESDIR}/5_15-audio-config"
	fi
	install_audio_configs elm "${audio_config_dir}"

	# Install rules to enable WoWLAN on startup.
	udev_dorules "${FILESDIR}/99-mwifiex-wowlan.rules"
}
