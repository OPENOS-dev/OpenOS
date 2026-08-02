# Copyright 1999-2025 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

# please keep this ebuild at EAPI 8 -- sys-apps/portage dep
EAPI=7

DISTUTILS_USE_PEP517=flit
PYTHON_COMPAT=( python3_{11..14} python3_{13,14}t pypy3_11 )

inherit distutils-r1 pypi

DESCRIPTION="Core utilities for Python packages"
HOMEPAGE="
	https://github.com/pypa/packaging/
	https://pypi.org/project/packaging/
"

LICENSE="|| ( Apache-2.0 BSD-2 )"
SLOT="0"
KEYWORDS="*"

RDEPEND="
	!<dev-python/setuptools-67
"
DEPEND="
	test? (
		dev-python/pretend[${PYTHON_USEDEP}]
	)
"

distutils_enable_tests pytest

python_test() {
	epytest --capture=no
}
