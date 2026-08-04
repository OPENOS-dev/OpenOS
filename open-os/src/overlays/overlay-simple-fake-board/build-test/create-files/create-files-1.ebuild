# Copyright 2023 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

DESCRIPTION="Creates files in the tmp and work dirs."
HOMEPAGE="http://www.chromium.org"

LICENSE="BSD-Google"
SLOT="${PN}/0"
KEYWORDS="*"

RDEPEND=""
DEPEND=""

S="${WORKDIR}"

src_compile() {
	echo "hi" > "${T}"/hi.txt
	echo "hello" > "${WORKDIR}"/hello.txt
}
