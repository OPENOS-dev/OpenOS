# Copyright 2021 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

# This ebuild only cares about its own FILESDIR and ebuild file, so it tracks
# the canonical empty project.
CROS_WORKON_COMMIT="d2d95e8af89939f893b1443135497c1f5572aebc"
CROS_WORKON_TREE="776139a53bc86333de8672a51ed7879e75909ac9"
CROS_WORKON_PROJECT="chromiumos/infra/build/empty-project"
CROS_WORKON_LOCALNAME="platform/empty-project"

inherit appid cros-unibuild cros-workon udev

DESCRIPTION="Ebuild which pulls in any necessary ebuilds as dependencies
or portage actions."

LICENSE="BSD-Google"
KEYWORDS="-* amd64 x86"
IUSE="brask-kernelnext brask-labstation midna"

# Add dependencies on other ebuilds from within this board overlay
RDEPEND="
	chromeos-base/chromeos-bsp-baseboard-brya:=
	chromeos-base/sof-binary:=
	chromeos-base/sof-topology:=
"
DEPEND="
	${RDEPEND}
	chromeos-base/chromeos-config:=
"

src_install() {
	if use brask-kernelnext; then
		doappid "{F33472D0-B468-422A-8A47-17613CD50732}" "CHROMEBOX"
	elif use brask-labstation; then
		doappid "{BB2F7FE3-BC46-4F46-8DA0-D50CDBAA4B20}" "CHROMEBOX"
	elif use midna; then
		doappid "{BB55D437-14B9-48D9-A6B2-1C6B994E307F}" "CHROMEBOX"
	else
		doappid "{94144292-1100-4882-AC53-51E04BB29F9E}" "CHROMEBOX"
	fi

	# Install audio config files
	unibuild_install_files audio-files

	udev_dorules "${FILESDIR}"/udev/*.rules

	exeinto "$(get_udevdir)"
	doexe "${FILESDIR}"/control_usb_nfc_beep.sh

	dosbin "${FILESDIR}/r8169_aspm_quirk.sh"
}
