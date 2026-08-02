# Copyright 1999-2024 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

DISTUTILS_USE_PEP517=setuptools
PYTHON_COMPAT=( python3_{8..12} )

inherit distutils-r1 pypi

DESCRIPTION="WebSockets state-machine based protocol implementation"
HOMEPAGE="
	https://github.com/python-hyper/wsproto/
	https://pypi.org/project/wsproto/
"

LICENSE="MIT"
SLOT="0"
KEYWORDS="*"

RDEPEND="
	>=dev-python/h11-0.9[${PYTHON_USEDEP}]
"

distutils_enable_tests pytest
