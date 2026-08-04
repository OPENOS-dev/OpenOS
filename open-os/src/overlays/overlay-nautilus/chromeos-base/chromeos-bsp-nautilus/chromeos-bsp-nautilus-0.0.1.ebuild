# Copyright 2017 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI="7"

inherit appid cros-audio-configs udev

DESCRIPTION="Ebuild which pulls in any necessary ebuilds as dependencies
or portage actions."

LICENSE="BSD-Google"
SLOT="0"
KEYWORDS="-* amd64 x86"
IUSE="kernel-5_4 modemfwd nautilus-kernelnext"
S="${WORKDIR}"

# Add dependencies on other ebuilds from within this board overlay
RDEPEND="modemfwd? ( chromeos-base/modemfwd-helpers )"
DEPEND="${RDEPEND}"

src_install() {
	if use nautilus-kernelnext; then
		doappid "{5E7BAA07-E48A-4C2D-A90D-52030BECE7E8}" "CHROMEBOOK"
	else
		doappid "{85F8FA82-F276-4EA6-8980-93FE091F6D25}" "CHROMEBOOK"
	fi
	# Install audio config files
	if use kernel-5_4; then
		local audio_config_dir="${FILESDIR}/legacy-skl-audio-config"
	else
		local audio_config_dir="${FILESDIR}/audio-config"
	fi
	install_audio_configs nautilus "${audio_config_dir}"

	# Install platform specific config files for power_manager.
	insinto "/usr/share/power_manager/board_specific"
	doins "${FILESDIR}"/powerd_prefs/*

	# Install udev rules for proximity sensor.
	udev_dorules "${FILESDIR}"/udev/*.rules

	# Override default cpufreq configuration
	insinto "/etc"
	doins "${FILESDIR}/cpufreq/cpufreq.conf"
}
