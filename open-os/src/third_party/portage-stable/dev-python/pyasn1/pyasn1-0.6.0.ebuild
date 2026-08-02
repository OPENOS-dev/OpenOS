# Copyright 1999-2024 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

DISTUTILS_USE_PEP517=setuptools
PYTHON_COMPAT=( python3_{8..12} )

inherit distutils-r1 pypi

DESCRIPTION="ASN.1 library for Python"
HOMEPAGE="
	https://pypi.org/project/pyasn1/
	https://github.com/pyasn1/pyasn1/
"

LICENSE="BSD-2"
SLOT="0"
KEYWORDS="*"

distutils_enable_tests unittest
distutils_enable_sphinx "docs/source"
