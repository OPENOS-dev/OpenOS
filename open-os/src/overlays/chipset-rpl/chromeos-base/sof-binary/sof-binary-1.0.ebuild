# Copyright 2024 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

EAPI=7

DESCRIPTION="Raptor Lake SOF firmware binary"
SRC_URI="gs://chromeos-localmirror/distfiles/${PN}-rpl-${PV}.tar.xz"

LICENSE="SOF"
SLOT="0"
KEYWORDS="*"

RDEPEND="
"
DEPEND="${RDEPEND}"

S=${WORKDIR}/${PN}-rpl-${PV}

src_install() {
	insinto /lib/firmware/intel/sof/community
	doins sof-rpl.ri
	doins sof-rpl.ldc
}
