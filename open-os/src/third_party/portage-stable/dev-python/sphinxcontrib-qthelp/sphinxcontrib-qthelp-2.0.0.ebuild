# Copyright 1999-2024 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

DISTUTILS_USE_PEP517=flit
PYTHON_COMPAT=( python3_{8..12} )

inherit distutils-r1 pypi

DESCRIPTION="Sphinx extension which outputs QtHelp documents"
HOMEPAGE="
	https://www.sphinx-doc.org/
	https://github.com/sphinx-doc/sphinxcontrib-qthelp/
	https://pypi.org/project/sphinxcontrib-qthelp/
"

LICENSE="BSD-2"
SLOT="0"
KEYWORDS="*"

PDEPEND="
	>=dev-python/sphinx-5[${PYTHON_USEDEP}]
"
BDEPEND="
	test? (
		>=dev-python/defusedxml-0.7.1[${PYTHON_USEDEP}]
		${PDEPEND}
	)
"

distutils_enable_tests pytest
