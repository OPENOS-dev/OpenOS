# Copyright 1999-2020 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

PYTHON_COMPAT=( python3_{6..12} )

inherit distutils-r1

MY_PN="Cycler"

DESCRIPTION="Composable style cycles"
HOMEPAGE="
	https://matplotlib.org/cycler/
	https://pypi.org/project/Cycler/
	https://github.com/matplotlib/cycler"
SRC_URI="mirror://pypi/${MY_PN:0:1}/${MY_PN}/${P}.tar.gz"

SLOT="0"
LICENSE="BSD"
KEYWORDS="*"
IUSE="test"

RDEPEND="dev-python/six[${PYTHON_USEDEP}]"
DEPEND="${RDEPEND}
	dev-python/setuptools[${PYTHON_USEDEP}]
	test? ( dev-python/nose[${PYTHON_USEDEP}] )
"

# Not shipped
# https://github.com/matplotlib/cycler/issues/21
RESTRICT=test

python_test() {
	nosetests --verbosity=3 || die
}
