# Copyright 2011 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7
CROS_WORKON_COMMIT="e67643c64a105f6f744b007eb857f381ace07e8e"
CROS_WORKON_TREE=("518b50f8b6d01e95cbd933487ed7c6452ac4acb3" "9aea86fd2a84958e4276f862da8e1541e392713a" "f91b6afd5f2ae04ee9a2c19109a3a4a36f7659e6")
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_LOCALNAME="platform2"
CROS_WORKON_DESTDIR="${S}"
CROS_WORKON_OUTOFTREE_BUILD=1
CROS_WORKON_SUBTREE="common-mk modem-utilities .gn"

PLATFORM_SUBDIR="modem-utilities"

inherit cros-workon tmpfiles platform

DESCRIPTION="Chromium OS modem utilities"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform2/+/HEAD/modem-utilities/"
SRC_URI=""
LICENSE="BSD-Google"
KEYWORDS="*"
# This package has no unittests.
RESTRICT="test"

RDEPEND="sys-apps/dbus"
DEPEND=""

src_install() {
	platform_src_install

	dotmpfiles tmpfiles.d/*.conf
}
