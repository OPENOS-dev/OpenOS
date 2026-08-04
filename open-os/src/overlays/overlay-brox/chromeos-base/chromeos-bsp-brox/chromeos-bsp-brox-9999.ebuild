# Copyright 2023 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

# This ebuild only cares about its own FILESDIR and ebuild file, so it tracks
# the canonical empty project.
CROS_WORKON_PROJECT="chromiumos/infra/build/empty-project"
CROS_WORKON_LOCALNAME="platform/empty-project"

inherit appid cros-unibuild cros-workon udev

DESCRIPTION="Ebuild which pulls in any necessary ebuilds as dependencies
or portage actions."

LICENSE="BSD-Google"
KEYWORDS="-* ~amd64 ~x86"
IUSE="
"

# Add dependencies on other ebuilds from within this board overlay
RDEPEND="
	chromeos-base/chromeos-bsp-baseboard-brox:=
	chromeos-base/sof-binary:=
	chromeos-base/sof-topology:=
	chromeos-base/chromeos-zephyr-ish:=
	chromeos-base/touch_updater:=
"
DEPEND="
	${RDEPEND}
	chromeos-base/chromeos-config:=
"

src_install() {
	doappid "{8FDEDD8D-EB7A-4B1A-9A0B-16CE6EDEA43B}" "CHROMEBOOK"

	# Install audio config files
	unibuild_install_files audio-files

	udev_dorules "${FILESDIR}"/udev/*.rules

	dosbin "${FILESDIR}/r8169_aspm_quirk.sh"
}
