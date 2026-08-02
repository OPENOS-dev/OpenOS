# Copyright 1999-2024 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

DISTUTILS_USE_PEP517=flit
PYTHON_COMPAT=( python3_{8..12} )

inherit distutils-r1 pypi

DESCRIPTION="Backported and Experimental Type Hints for Python 3.7+"
HOMEPAGE="
	https://pypi.org/project/typing-extensions/
	https://github.com/python/typing_extensions/
"

LICENSE="PSF-2"
SLOT="0"
KEYWORDS="*"

distutils_enable_tests unittest

python_test() {
	cd src || die
	eunittest
}
