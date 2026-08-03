# Copyright 1999-2024 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

DISTUTILS_USE_PEP517=setuptools
PYTHON_COMPAT=( python3_{8..12} )

inherit distutils-r1 pypi

DESCRIPTION="Pure Python module for getting image size from png/jpeg/jpeg2000/gif files"
HOMEPAGE="
	https://github.com/shibukawa/imagesize_py/
	https://pypi.org/project/imagesize/
"

LICENSE="MIT"
SLOT="0"
KEYWORDS="*"

distutils_enable_tests pytest

EPYTEST_DESELECT=(
	# requires Internet
	test/test_get_filelike.py::test_get_filelike
)
