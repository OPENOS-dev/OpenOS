# Copyright 2021 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

DESCRIPTION="Camera support library for Linux virtual package"

LICENSE="LGPL-2.1+"
SLOT="0"
KEYWORDS="*"

IUSE="libcamera-mtkisp7 libcamera-intelipu7 libcamera-reven"

RDEPEND="
	libcamera-mtkisp7? ( media-libs/libcamera-mtkisp7 )
	libcamera-intelipu7? ( media-libs/libcamera-intelipu7 )
	libcamera-reven? ( media-libs/libcamera-reven )
	!libcamera-mtkisp7? ( !libcamera-intelipu7? ( !libcamera-reven? ( media-libs/libcamera-upstream ) ) )
"
