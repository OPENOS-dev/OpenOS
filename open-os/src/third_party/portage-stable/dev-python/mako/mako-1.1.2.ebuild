# Copyright 1999-2020 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

DISTUTILS_USE_SETUPTOOLS=rdepend
PYTHON_COMPAT=( python3_{6..12} )

inherit distutils-r1 eutils

MY_P=${P^}
DESCRIPTION="A Python templating language"
HOMEPAGE="https://www.makotemplates.org/ https://pypi.org/project/Mako/"
SRC_URI="mirror://pypi/${MY_P:0:1}/${PN^}/${MY_P}.tar.gz"
S="${WORKDIR}/${MY_P}"

LICENSE="MIT"
SLOT="0"
KEYWORDS="*"
IUSE="doc"

RDEPEND=">=dev-python/markupsafe-0.9.2[${PYTHON_USEDEP}]"

PATCHES=(
	"${FILESDIR}"/mako-1.1.1-pypy3-test.patch
)

distutils_enable_tests pytest

src_prepare() {
	# seriously?
	sed -i -e 's:from nose import:from unittest import:' \
		test/__init__.py || die

	distutils-r1_src_prepare
}

python_install_all() {
	rm -r doc/build || die

	use doc && local HTML_DOCS=( doc/. )
	distutils-r1_python_install_all
}

pkg_postinst() {
	elog "Optional dependencies:"
	optfeature "caching support" dev-python/beaker
}
