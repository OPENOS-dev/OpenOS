# Copyright 2022-2024 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

DISTUTILS_USE_PEP517=standalone
PYTHON_COMPAT=( python3_{8..12} )

inherit distutils-r1

DESCRIPTION="A PEP 518 build backend combining flit with setuptools_scm"
HOMEPAGE="
	https://gitlab.com/WillDaSilva/flit_scm/
	https://pypi.org/project/flit-scm/
"
SRC_URI="
	https://gitlab.com/WillDaSilva/flit_scm/-/archive/${PV}/${P}.tar.bz2
"

LICENSE="MIT"
SLOT="0"
KEYWORDS="*"

RDEPEND="
	>=dev-python/flit-core-3.5.0[${PYTHON_USEDEP}]
	>=dev-python/setuptools-scm-6.4.2[${PYTHON_USEDEP}]
	$(python_gen_cond_dep '
		<dev-python/tomli-3[${PYTHON_USEDEP}]
		>=dev-python/tomli-2[${PYTHON_USEDEP}]
	' 3.8 3.9 3.10 )
"
BDEPEND="
	${RDEPEND}
"

src_prepare() {
	# unpin deps
	sed -i -e 's:~=[0-9.]*::' pyproject.toml || die
	distutils-r1_src_prepare
}

export SETUPTOOLS_SCM_PRETEND_VERSION=${PV}
