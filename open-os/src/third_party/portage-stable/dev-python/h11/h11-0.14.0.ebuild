# Copyright 1999-2024 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

DISTUTILS_USE_PEP517=setuptools
PYTHON_COMPAT=( python3_{8..12} )

inherit distutils-r1 pypi

DESCRIPTION="A pure-Python implementation of HTTP/1.1 inspired by hyper-h2"
HOMEPAGE="
	https://h11.readthedocs.io/en/latest/
	https://github.com/python-hyper/h11/
	https://pypi.org/project/h11/
"

LICENSE="MIT"
SLOT="0"
KEYWORDS="*"

distutils_enable_tests pytest
