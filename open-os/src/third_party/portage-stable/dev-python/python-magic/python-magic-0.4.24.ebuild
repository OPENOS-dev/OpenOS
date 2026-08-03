# Copyright 1999-2021 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

PYTHON_COMPAT=( python3_{8..11} pypy3 )

inherit distutils-r1

DESCRIPTION="Access the libmagic file type identification library"
HOMEPAGE="https://github.com/ahupp/python-magic"
SRC_URI="
	https://github.com/ahupp/python-magic/archive/${PV}.tar.gz
		-> ${P}.gh.tar.gz"

LICENSE="BSD-2 MIT"
SLOT="0"
KEYWORDS="*"
IUSE="test"
RESTRICT="!test? ( test )"

RDEPEND="sys-apps/file[-python]"
BDEPEND="
	test? ( sys-apps/file )
"

python_test() {
	local -x LC_ALL=en_US.UTF-8
	"${EPYTHON}" test/test.py -v || die "Tests fail with ${EPYTHON}"
}
