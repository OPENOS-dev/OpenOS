# Copyright 2017 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI="7"

inherit cros-toolchain-funcs eutils

DESCRIPTION="U2F reference code and test tools"
HOMEPAGE="https://github.com/google/u2f-ref-code"

GIT_SHA1="8f37b6e2265717cbc2acd0a9c4144c7fcd09af6c"
MY_P=${PN}-8f37b6e
SRC_URI="http://github.com/google/u2f-ref-code/archive/${GIT_SHA1}.tar.gz -> ${MY_P}.tar.gz
		https://android.googlesource.com/platform/system/core/+archive/lollipop-release.tar.gz -> android-system-core-lollipop-release.tar.gz"

LICENSE="BSD-Google"
SLOT="0"
KEYWORDS="*"
IUSE=""

RDEPEND="dev-libs/hidapi
	virtual/libudev"
DEPEND="${RDEPEND}"

S="${WORKDIR}/${PN}-${GIT_SHA1}/u2f-tests/HID"

src_prepare() {
	ln -s "${WORKDIR}" "${S}/core" || die
	default
}

src_configure() {
	tc-export CC CXX PKG_CONFIG
}

src_compile() {
	emake UNAME=Linux {U2F,HID}Test
}

src_install() {
	dobin {U2F,HID}Test
}
