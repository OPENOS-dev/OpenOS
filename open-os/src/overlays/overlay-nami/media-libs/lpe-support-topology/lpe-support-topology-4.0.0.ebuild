# Copyright (c) 2018 The Chromium OS Authors. All rights reserved.
# Distributed under the terms of the GNU General Public License v2

EAPI="7"

inherit cros-binary

DESCRIPTION="Topology binary files used to support/configure LPE Audio"
LICENSE="LICENCE.adsp_sst"
SLOT="0"
SRC_URI="gs://chromeos-localmirror/distfiles/${PN}-nami-${PV}.tbz2"
KEYWORDS="-* x86 amd64"

RDEPEND="
	media-libs/kbl-hotword-support
"

DEPEND="${RDEPEND}"
IUSE="kernel-5_4"

S=${WORKDIR}

src_install() {
	# Archive contains both topologies. The “old” one for kernel 5.4 (single file 9d71-GOOGLE-NAMIMAX-0-tplg.bin)
	# and “new” for kernel 6.6 based on AVS. The AVS based is a set of files under “intel” directory.
	# Clean up (remove) the unnecessary part and install remains into rootfs
	FWPATH=lib/firmware
	if use kernel-5_4; then
		rm -rf ${FWPATH}/intel
	else
		rm -f ${FWPATH}/9d71-GOOGLE-NAMIMAX-0-tplg.bin
	fi
	insinto /
	doins -r ./*

}
