# Copyright 2014-2023 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

if [[ ${PV} == 9999* ]]; then
	inherit git-r3
	EGIT_REPO_URI="https://anongit.gentoo.org/git/proj/gentoo-functions.git"
else
	SRC_URI="https://gitweb.gentoo.org/proj/gentoo-functions.git/snapshot/${P}.tar.bz2"
	KEYWORDS="*"
fi

inherit toolchain-funcs flag-o-matic

DESCRIPTION="Base functions required by all Gentoo systems"
HOMEPAGE="https://gitweb.gentoo.org/proj/gentoo-functions.git"

LICENSE="GPL-2"
SLOT="0"

# Specifically needs GNU find, as well.
RDEPEND=">=sys-apps/findutils-4.9"

src_configure() {
	tc-export CC
	append-lfs-flags
	export ROOTPREFIX="${EPREFIX}"
	export PREFIX="${EPREFIX}/usr"
}
