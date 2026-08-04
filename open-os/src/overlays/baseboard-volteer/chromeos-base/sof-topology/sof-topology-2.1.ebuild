# Copyright 2018 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

EAPI="7"

DESCRIPTION="Topology file needed to run SOF."

# Version of the package to be downloaded will depend on version of the applied
# kernel, which is fixed to 1.15 for kernel-5.4 (before migration) otherwise
# the latest version (after migration).
PREMV="1.15"
SRC_URI="
	kernel-5_4? (
		gs://chromeos-localmirror/distfiles/${PN}-volteer-${PREMV}.tar.bz2
	)
	!kernel-5_4? (
		gs://chromeos-localmirror/distfiles/${PN}-volteer-${PV}.tar.bz2
	)
"

LICENSE="SOF"
SLOT="0"
KEYWORDS="*"
IUSE="kernel-5_4"

S="${WORKDIR}"

src_prepare() {
	if use kernel-5_4; then
		S="${S}"/"${PN}-volteer-${PREMV}"
	else
		S="${S}"/"${PN}-volteer-${PV}"
	fi
	eapply_user
}

src_install() {
	insinto /lib/firmware/intel/sof-tplg
	doins ./*.tplg
	insinto /lib/firmware/intel/sof-tplg/pdm1
	doins pdm1/*.tplg
	insinto /lib/firmware/intel/sof-tplg/pdm1-drceq
	doins pdm1-drceq/*.tplg
	dodoc README
}
