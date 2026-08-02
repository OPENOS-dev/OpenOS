# Copyright 1999-2024 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

DISTUTILS_USE_PEP517=setuptools
PYTHON_COMPAT=( python3_{8..12} )

inherit distutils-r1 pypi

DESCRIPTION="C++ header library which makes it easier to write Python extension modules"
HOMEPAGE="
	https://github.com/nucleic/cppy/
	https://pypi.org/project/cppy/
"

LICENSE="BSD"
SLOT="0"
KEYWORDS="*"

# bug #836765 for setuptools >= dep
BDEPEND=">=dev-python/setuptools-61.2[${PYTHON_USEDEP}]"

distutils_enable_tests pytest
