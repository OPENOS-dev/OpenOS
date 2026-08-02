# Copyright 2023 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7
CROS_WORKON_LOCALNAME="platform2"
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_DESTDIR="${S}/platform2"
CROS_WORKON_SUBTREE="common-mk sandboxing-codelab .gn"

CROS_WORKON_OUTOFTREE_BUILD="1"

PLATFORM_SUBDIR="sandboxing-codelab"

inherit cros-workon platform

DESCRIPTION="Sandboxing/security codelab for ChromiumOS"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform2/+/HEAD/sandboxing-codelab/"

LICENSE="BSD-Google"
KEYWORDS="~*"
IUSE=""

src_install() {
	platform_src_install
}
