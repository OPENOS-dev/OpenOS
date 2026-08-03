# Copyright 1999-2023 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

DISTUTILS_USE_PEP517=setuptools
PYTHON_COMPAT=( python3_{6..11} pypy3 )
PYTHON_REQ_USE="threads(+)"

inherit distutils-r1 pypi

DESCRIPTION="Python module for spawning child apps and responding to expected patterns"
HOMEPAGE="
	https://pexpect.readthedocs.io/
	https://pypi.org/project/pexpect/
	https://github.com/pexpect/pexpect/
"

LICENSE="MIT"
SLOT="0"
KEYWORDS="*"
IUSE="examples"

RDEPEND="
	>=dev-python/ptyprocess-0.5[${PYTHON_USEDEP}]
"

PATCHES=(
	"${FILESDIR}"/${P}-sphinx-3.patch
	"${FILESDIR}"/${P}-fix-PS1.patch
	"${FILESDIR}"/${P}-py311.patch
)

distutils_enable_tests pytest
distutils_enable_sphinx doc

src_test() {
	# workaround new readline defaults
	echo "set enable-bracketed-paste off" > "${T}"/inputrc || die
	local -x INPUTRC="${T}"/inputrc
	distutils-r1_src_test
}

python_install_all() {
	if use examples; then
		dodoc -r examples
		docompress -x /usr/share/doc/${PF}/examples
	fi
	distutils-r1_python_install_all
}
