# Copyright 2022 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

# This ebuild only cares about its own FILESDIR and ebuild file, so it tracks
# the canonical empty project.
CROS_WORKON_COMMIT="d2d95e8af89939f893b1443135497c1f5572aebc"
CROS_WORKON_TREE="776139a53bc86333de8672a51ed7879e75909ac9"
CROS_WORKON_PROJECT="chromiumos/infra/build/empty-project"
CROS_WORKON_LOCALNAME="../platform/empty-project"
CROS_WORKON_OUTOFTREE_BUILD=1

inherit cros-workon cros-fwupd

DESCRIPTION="Installs eMMC firmware update files used by fwupd."
HOMEPAGE="https://fwupd.org/downloads"

KEYWORDS="*"

FILENAMES=(
	"c94adae3cc18a778c0fa9c9eaf04b94d42253580805bf81c4a1b9335824bee38-GenesysLogic_Google_Servo_GL3590_64.18.cab"
)
SRC_URI="${FILENAMES[*]/#/${CROS_FWUPD_URL}/}"
LICENSE="LVFS-Vendor-Agreement-v1"

DEPEND=""
RDEPEND="sys-apps/fwupd"

# There is a chicken and egg problem between PARIS and the labstation, currently PARIS calls for v 64.17 to be installed
# however v 64.18 is required for the new servo devices being manufactured by Acroname ( anything with the GL3590 S2 chip )
# if I update PARIS before everything makes it to labstation then some labstations will fail as they only have the v64.17
# for now link 64.18 to 64.17, then everything gets updated when the labstation pushes.  Eventually update PARIS and then
# remove this link after PARIS is pushed.
src_install() {
	cros-fwupd_src_install
	if ! use remote; then
		dosym "c94adae3cc18a778c0fa9c9eaf04b94d42253580805bf81c4a1b9335824bee38-GenesysLogic_Google_Servo_GL3590_64.18.cab" \
			"/usr/share/fwupd/remotes.d/vendor/firmware/be2c9146ff4cfac5d647376c39ce0b78151e9f1a785a287e93ac3968aff2ed50-GenesysLogic_GL3590_64.17.cab"
	fi
}
