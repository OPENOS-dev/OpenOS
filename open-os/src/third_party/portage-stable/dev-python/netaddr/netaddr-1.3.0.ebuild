# Copyright 1999-2024 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

DISTUTILS_USE_PEP517=setuptools
PYTHON_COMPAT=( python3_{8..12} )

inherit distutils-r1 pypi

DESCRIPTION="Network address representation and manipulation library"
HOMEPAGE="
	https://github.com/netaddr/netaddr/
	https://pypi.org/project/netaddr/
	https://netaddr.readthedocs.io/
"

LICENSE="BSD"
SLOT="0"
KEYWORDS="*"

BDEPEND="
	test? (
		dev-python/packaging[${PYTHON_USEDEP}]
	)
"

distutils_enable_sphinx docs/source \
	dev-python/furo \
	dev-python/sphinx-issues
distutils_enable_tests pytest
