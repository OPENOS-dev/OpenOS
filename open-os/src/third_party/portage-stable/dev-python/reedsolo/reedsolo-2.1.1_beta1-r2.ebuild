# Copyright 2021-2025 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

DISTUTILS_EXT=1
DISTUTILS_USE_PEP517=setuptools
PYTHON_COMPAT=( python3_{11..14} )

inherit distutils-r1 pypi

DESCRIPTION="Python Reed Solomon encoder/decoder"
HOMEPAGE="
	https://github.com/tomerfiliba-org/reedsolomon/
	https://pypi.org/project/reedsolo/
"

LICENSE="|| ( Unlicense MIT-0 )"
SLOT="0"
KEYWORDS="-* amd64"
IUSE="+native-extensions"

distutils_enable_tests pytest

src_prepare() {
	sed -i -e '/pytest-cov/d' pyproject.toml || die
	distutils-r1_src_prepare
}

python_compile() {
	local DISTUTILS_ARGS=()
	distutils-r1_python_compile
}
