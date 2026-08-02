# Copyright 1999-2023 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

DISTUTILS_USE_PEP517=setuptools
PYTHON_COMPAT=( python3_{8..12} )

inherit distutils-r1 pypi

DESCRIPTION="Atomic file writes"
HOMEPAGE="
	https://github.com/untitaker/python-atomicwrites/
	https://pypi.org/project/atomicwrites/
"

LICENSE="MIT"
SLOT="0"
KEYWORDS="*"

distutils_enable_tests pytest
distutils_enable_sphinx docs \
	dev-python/sphinx-rtd-theme
