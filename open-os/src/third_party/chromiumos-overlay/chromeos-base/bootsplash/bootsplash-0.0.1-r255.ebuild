# Copyright 2022 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

EAPI="7"

CROS_WORKON_COMMIT="e67643c64a105f6f744b007eb857f381ace07e8e"
CROS_WORKON_TREE=("518b50f8b6d01e95cbd933487ed7c6452ac4acb3" "db59d4933796b470cd3105e530b6132ebaa277fb" "4934b6b332f2a3db7a26bad9f888607a4f12b440" "f91b6afd5f2ae04ee9a2c19109a3a4a36f7659e6")
CROS_WORKON_INCREMENTAL_BUILD=1
CROS_WORKON_LOCALNAME="platform2"
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_DESTDIR="${S}/platform2"
CROS_WORKON_SUBTREE="common-mk bootsplash libec .gn"

PLATFORM_SUBDIR="bootsplash"

inherit cros-workon platform user

DESCRIPTION="Frecon-based boot splash service"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform2/+/HEAD/bootsplash"

LICENSE="BSD-Google"
SLOT="0/0"
KEYWORDS="*"

DEPEND="
	chromeos-base/bootstat:=
	chromeos-base/session_manager-client:=
	chromeos-base/system_api:=
	dev-libs/re2:=
"

RDEPEND="
	${DEPEND}
	sys-apps/frecon
"

pkg_preinst() {
	enewuser "bootsplash"
	enewgroup "bootsplash"
}

src_install() {
	platform_src_install

	dobin "${OUT}/bootsplash"
}
