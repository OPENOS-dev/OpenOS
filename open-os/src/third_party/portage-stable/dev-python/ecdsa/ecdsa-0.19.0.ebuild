# Copyright 1999-2024 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

DISTUTILS_USE_PEP517=setuptools
PYTHON_COMPAT=( python3_{8..12} )

inherit distutils-r1 pypi

DESCRIPTION="ECDSA cryptographic signature library in pure Python"
HOMEPAGE="
	https://github.com/tlsfuzzer/python-ecdsa/
	https://pypi.org/project/ecdsa/
"

LICENSE="MIT"
SLOT="0"
KEYWORDS="*"

RDEPEND="
	$(python_gen_cond_dep '
		dev-python/gmpy[${PYTHON_USEDEP}]
	' 'python*')
	dev-python/six[${PYTHON_USEDEP}]
"
BDEPEND="
	test? (
		dev-python/hypothesis[${PYTHON_USEDEP}]
	)
"

distutils_enable_tests pytest
