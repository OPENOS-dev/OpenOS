# Copyright 1999-2022 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

DISTUTILS_USE_PEP517=setuptools
PYTHON_COMPAT=( python3_{8..12} )

inherit distutils-r1

DESCRIPTION="Correctly inflect words and numbers"
HOMEPAGE="
	https://pypi.org/project/inflect/
	https://github.com/jaraco/inflect/
"
SRC_URI="mirror://pypi/${PN:0:1}/${PN}/${P}.tar.gz"

LICENSE="MIT"
SLOT="0"
KEYWORDS="*"

RDEPEND="
	>=dev-python/pydantic-1.9.1[${PYTHON_USEDEP}]
"
BDEPEND="
	>=dev-python/setuptools-scm-3.4.1[${PYTHON_USEDEP}]
"

distutils_enable_tests pytest
