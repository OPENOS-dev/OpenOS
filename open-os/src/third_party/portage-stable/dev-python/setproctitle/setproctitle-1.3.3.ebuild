# Copyright 1999-2024 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

DISTUTILS_EXT=1
DISTUTILS_USE_PEP517=setuptools
PYTHON_COMPAT=( python3_{8..12} )

inherit distutils-r1 pypi

DESCRIPTION="Allow customization of the process title"
HOMEPAGE="
	https://github.com/dvarrazzo/py-setproctitle/
	https://pypi.org/project/setproctitle/
"

LICENSE="BSD"
SLOT="0"
KEYWORDS="*"

distutils_enable_tests pytest
