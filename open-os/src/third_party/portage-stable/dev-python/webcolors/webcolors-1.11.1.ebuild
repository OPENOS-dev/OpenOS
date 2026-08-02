# Copyright 1999-2020 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

PYTHON_COMPAT=( python3_{6..12} )

inherit distutils-r1

DESCRIPTION="Color names and value formats defined by the HTML and CSS specifications"
HOMEPAGE="https://pypi.org/project/webcolors/ https://github.com/ubernostrum/webcolors"
SRC_URI="mirror://pypi/${PN:0:1}/${PN}/${P}.tar.gz"

SLOT="0"
LICENSE="BSD"
KEYWORDS="*"

BDEPEND="dev-python/six[${PYTHON_USEDEP}]"

distutils_enable_tests nose
