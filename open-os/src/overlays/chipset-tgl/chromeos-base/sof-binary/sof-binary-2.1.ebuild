# Copyright 2018 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2.

EAPI="7"

DESCRIPTION="Tiger Lake SOF firmware binary"

# Version of the package to be downloaded will depend on version of the applied
# kernel, which is fixed to 1.24 for kernel-5.4 (before migration) otherwise
# the latest version (after migration).
PREMV="1.24"
SRC_URI="
	kernel-5_4? (
		gs://chromeos-localmirror/distfiles/${PN}-tgl-${PREMV}.tar.bz2
	)
	!kernel-5_4? (
		gs://chromeos-localmirror/distfiles/${PN}-tgl-${PV}.tar.bz2
	)
"

LICENSE="SOF"
SLOT="0"
KEYWORDS="*"
IUSE="kernel-5_4"

RDEPEND="
	media-libs/tgl-drceq-param
	media-libs/tgl-dsm-param
	media-libs/tgl-hotword-support
"
DEPEND="${RDEPEND}"

S="${WORKDIR}"

src_prepare() {
	if use kernel-5_4; then
		S="${S}"/"${PN}-tgl-${PREMV}"
	else
		S="${S}"/"${PN}-tgl-${PV}"
	fi
	eapply_user
}

src_install() {
	insinto /lib/firmware/intel/sof/community
	doins sof-tgl.ri
	dodoc README
}
