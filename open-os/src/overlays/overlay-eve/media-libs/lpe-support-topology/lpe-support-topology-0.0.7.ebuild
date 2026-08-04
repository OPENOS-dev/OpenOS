# Copyright 2017 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI="7"

DESCRIPTION="Topology binary files used to support/configure LPE Audio"
LICENSE="LICENCE.adsp_sst"
SLOT="0"
SRC_URI="gs://chromeos-localmirror/distfiles/${PN}-eve-${PV}.tbz2"
KEYWORDS="-* x86 amd64"

RDEPEND="
	media-libs/kbl-tuning-support
	media-libs/kbl-dsm-param
"
IUSE="kernel-6_6"
S=${WORKDIR}

src_install() {
	# Archive contains both topologies. The “old” one for kernel 5.4 (single file 9d71-GOOGLE-EVEMAX-0-tplg.bin)
	# and “new” for kernel 6.6 based on AVS. The AVS based is a set of files under “intel” directory.
	# Clean up (remove) the unnecessary part and install remains into rootfs
	FWPATH=lib/firmware
	if use kernel-6_6; then
		rm -f ${FWPATH}/9d71-GOOGLE-EVEMAX-0-tplg.bin
	else
		rm -rf ${FWPATH}/intel
	fi
	insinto /
	doins -r ./*
}
