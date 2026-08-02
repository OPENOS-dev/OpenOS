# Copyright 2023 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI="7"

DESCRIPTION="Log collection utility for Quectel modems"
HOMEPAGE="https://github.com/quectel-official/QLog"
GIT_SHA1="81527baf70345aabf02402f4961e368034a92258"
SRC_URI="${HOMEPAGE}/archive/${GIT_SHA1}.tar.gz -> QLog-${GIT_SHA1}.tar.gz"

LICENSE="Apache-2.0"
SLOT="0"
KEYWORDS="*"
IUSE=""

RDEPEND="
	virtual/libudev:=
"
DEPEND="${RDEPEND}"

S="${WORKDIR}/QLog-${GIT_SHA1}"

src_install() {
	default

	insinto /usr/share/qlog
	doins "${S}"/conf/default.cfg
}
