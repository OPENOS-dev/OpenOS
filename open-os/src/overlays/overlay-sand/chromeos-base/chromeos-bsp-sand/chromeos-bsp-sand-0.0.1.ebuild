# Copyright 2017 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI="7"

inherit appid

DESCRIPTION="Ebuild which pulls in any necessary ebuilds as dependencies
or portage actions."

LICENSE="BSD-Google"
SLOT="0"
KEYWORDS="-* amd64 x86"
IUSE="sand-kernelnext"
S="${WORKDIR}"

# Add dependencies on other ebuilds from within this board overlay
RDEPEND="
	chromeos-base/chromeos-bsp-baseboard-reef
"
DEPEND="${RDEPEND}"

src_install() {
	if use sand-kernelnext; then
		doappid "{48AA7FE1-4811-46BA-BFF5-B43257B13B7C}" "CHROMEBOOK"
	else
		doappid "{533B7092-5828-48E6-9F77-C7ADCBF8F7E9}" "CHROMEBOOK"
	fi
}
