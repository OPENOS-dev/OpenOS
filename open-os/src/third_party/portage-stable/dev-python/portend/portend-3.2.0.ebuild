# Copyright 1999-2024 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

DISTUTILS_USE_PEP517=setuptools
PYTHON_COMPAT=( python3_{8..12} )

inherit distutils-r1 pypi

DESCRIPTION="TCP port monitoring utilities"
HOMEPAGE="
	https://github.com/jaraco/portend/
	https://pypi.org/project/portend/
"

LICENSE="MIT"
SLOT="0"
KEYWORDS="*"

RDEPEND="
	>=dev-python/tempora-1.8[${PYTHON_USEDEP}]
"
BDEPEND="
	dev-python/setuptools-scm[${PYTHON_USEDEP}]
"

distutils_enable_tests pytest
