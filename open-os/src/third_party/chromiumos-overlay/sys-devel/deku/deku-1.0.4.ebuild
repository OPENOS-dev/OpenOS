# Copyright 2025 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

EAPI="7"

inherit cros-toolchain-funcs

DESCRIPTION="DEKU is a utility for Linux kernel developers to quickly apply changes from the source code to the running kernel without the need for a system reboot"
HOMEPAGE="https://github.com/google/deku"
SRC_URI="https://github.com/google/${PN}/archive/refs/tags/${PV}.tar.gz -> ${P}.tar.gz"

LICENSE="BSD-Google"
SLOT="0"
KEYWORDS="*"
IUSE=""

DEPEND="
	sys-libs/binutils-libs:=
	virtual/libelf:=
"
RDEPEND="${DEPEND}"
BDEPEND="
	dev-lang/go
"
src_configure() {
	tc-export AR CC
}

src_install() {
	dobin deku
	dodoc README.md
}
