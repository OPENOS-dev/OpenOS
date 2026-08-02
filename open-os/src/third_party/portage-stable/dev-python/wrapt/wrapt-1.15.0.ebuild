# Copyright 1999-2023 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

DISTUTILS_USE_PEP517=setuptools
PYTHON_COMPAT=( python3_{8..11} pypy3 )

inherit distutils-r1

DESCRIPTION="Module for decorators, wrappers and monkey patching"
HOMEPAGE="
	https://github.com/GrahamDumpleton/wrapt/
	https://pypi.org/project/wrapt/
"
SRC_URI="
	https://github.com/GrahamDumpleton/wrapt/archive/${PV}.tar.gz
		-> ${P}.gh.tar.gz
"

LICENSE="BSD"
SLOT="0"
KEYWORDS="*"

distutils_enable_tests pytest
distutils_enable_sphinx docs dev-python/sphinx-rtd-theme

python_compile() {
	local -x WRAPT_INSTALL_EXTENSIONS=true
	distutils-r1_python_compile
}

python_test() {
	local -x PYTEST_DISABLE_PLUGIN_AUTOLOAD=1
	epytest
}
