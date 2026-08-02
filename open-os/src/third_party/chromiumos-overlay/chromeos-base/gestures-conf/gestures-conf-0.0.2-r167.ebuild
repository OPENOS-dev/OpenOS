# Copyright 2014 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7
CROS_WORKON_COMMIT="979fc788d118a91f86cd6c7d10b5ccd93ac9c3c2"
CROS_WORKON_TREE="80eb7b91d652e02f05b2fe074962d4fcee5d9cba"
CROS_WORKON_LOCALNAME="platform/xorg-conf"
CROS_WORKON_PROJECT="chromiumos/platform/xorg-conf"
CROS_WORKON_OUTOFTREE_BUILD=1

inherit cros-workon

DESCRIPTION="Gestures library configuration files"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform/xorg-conf/"
SRC_URI=""

LICENSE="BSD-Google"
KEYWORDS="*"

RDEPEND="!chromeos-base/touchpad-linearity"
DEPEND=""

src_install() {
	insinto /etc/gesture

	doins 20-mouse.conf
	doins 40-touchpad-cmt.conf
}
