# Copyright 1999-2021 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

DESCRIPTION="Framework for testing other programs"
HOMEPAGE="https://www.gnu.org/software/dejagnu/"
SRC_URI="mirror://gnu/${PN}/${P}.tar.gz"

LICENSE="GPL-3"
SLOT="0"
KEYWORDS="*"
IUSE="test"

#RESTRICT="!test? ( test )"
RESTRICT="test" # needs fixing

RDEPEND="dev-tcltk/expect"
BDEPEND="app-alternatives/awk"
#DEPEND="test? ( dev-tcltk/expect )"
