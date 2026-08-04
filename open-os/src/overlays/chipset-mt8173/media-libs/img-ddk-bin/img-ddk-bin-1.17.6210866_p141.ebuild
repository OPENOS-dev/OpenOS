# Copyright 2015 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI="7"

inherit unpacker

DESCRIPTION="PowerVR Rogue GLES2 hardware driver, binary only install"
SRC_URI="
arm? ( http://commondatastorage.googleapis.com/chromeos-localmirror/distfiles/img-ddk-oak-${PV}.run )
arm64? ( http://commondatastorage.googleapis.com/chromeos-localmirror/disfiles/img-ddk-oak64-${PV}.run )
"

LICENSE="Google-TOS"
SLOT="0"
KEYWORDS="-* arm64 arm"

RDEPEND="
	!media-libs/img-ddk
	x11-libs/libdrm
	"

S=${WORKDIR}

src_install() {
	cp -pPR "${S}"/* "${D}/" || die "Install failed!"
}
