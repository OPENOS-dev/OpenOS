# Copyright 1999-2024 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

DISTUTILS_EXT=1
DISTUTILS_USE_PEP517=setuptools
PYTHON_COMPAT=( python3_{8..12} )

inherit distutils-r1 pypi

DESCRIPTION="Persistent/Functional/Immutable data structures"
HOMEPAGE="
	https://github.com/tobgu/pyrsistent/
	https://pypi.org/project/pyrsistent/
"

LICENSE="MIT"
SLOT="0"
KEYWORDS="*"
IUSE="+native-extensions"

DEPEND="${PYTHON_DEPS}"
BDEPEND="
	test? (
		dev-python/hypothesis[${PYTHON_USEDEP}]
	)
"

distutils_enable_tests pytest

src_configure() {
	if ! use native-extensions; then
		export PYRSISTENT_SKIP_EXTENSION=1
	fi
}
