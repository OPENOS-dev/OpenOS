# Copyright 1999-2024 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

DISTUTILS_USE_PEP517=setuptools
PYTHON_COMPAT=( python3_{8..12} )
PYTHON_REQ_USE="threads(+)"

inherit distutils-r1

DESCRIPTION="A full-featured template engine for Python"
HOMEPAGE="
	https://palletsprojects.com/p/jinja/
	https://github.com/pallets/jinja/
	https://pypi.org/project/Jinja2/
"
SRC_URI="
	https://github.com/pallets/jinja/archive/${PV}.tar.gz
		-> jinja-${PV}.gh.tar.gz
"

S="${WORKDIR}/jinja-${PV}"

LICENSE="BSD"
SLOT="0"
KEYWORDS="*"
IUSE="examples"

RDEPEND="
	>=dev-python/markupsafe-2.0.0[${PYTHON_USEDEP}]
	!dev-python/jinja
"

distutils_enable_sphinx docs \
	dev-python/sphinx-issues \
	dev-python/pallets-sphinx-themes
distutils_enable_tests pytest

# XXX: handle Babel better?

src_prepare() {
	# avoid unnecessary dep on extra sphinxcontrib modules
	sed -i '/sphinxcontrib.log_cabinet/ d' docs/conf.py || die

	distutils-r1_src_prepare
}

python_install_all() {
	if use examples ; then
		docinto examples
		dodoc -r examples/.
	fi

	distutils-r1_python_install_all
}

pkg_postinst() {
	if ! has_version dev-python/Babel; then
		elog "For i18n support, please emerge dev-python/Babel."
	fi
}
