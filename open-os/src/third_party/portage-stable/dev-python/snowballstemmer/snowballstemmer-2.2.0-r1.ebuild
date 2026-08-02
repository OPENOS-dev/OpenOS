# Copyright 1999-2024 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

DISTUTILS_USE_PEP517=setuptools
PYTHON_COMPAT=( python3_{8..12} )
inherit distutils-r1 pypi

DESCRIPTION="Stemmer algorithms generated from Snowball algorithms"
HOMEPAGE="
	https://snowballstem.org/
	https://github.com/snowballstem/snowball
	https://pypi.org/project/snowballstemmer/
"

LICENSE="BSD"
SLOT="0"
KEYWORDS="*"
