# Copyright 1999-2020 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

DISTUTILS_USE_PEP517=setuptools
PYTHON_COMPAT=( python3_{8..11} pypy3 )

inherit distutils-r1

MY_P=certifi-shim-${PV}
DESCRIPTION="Thin replacement for certifi using system certificate store"
HOMEPAGE="
	https://github.com/mgorny/certifi-shim
	https://pypi.org/project/certifi"
SRC_URI="
	https://github.com/mgorny/certifi-shim/archive/v${PV}.tar.gz
		-> ${MY_P}.tar.gz"
S=${WORKDIR}/${MY_P}

LICENSE="CC0-1.0"
SLOT="0"
KEYWORDS="*"

RDEPEND="app-misc/ca-certificates"

distutils_enable_tests unittest

src_prepare() {
	sed -i -e "s^/etc^${EPREFIX}/etc^" certifi/core.py || die
	distutils-r1_src_prepare
}
