# Copyright 1999-2021 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

inherit autotools

DESCRIPTION="Manages the /usr/bin/vi symlink"
HOMEPAGE="https://wiki.gentoo.org/wiki/Project:Vim"
SRC_URI="https://gitweb.gentoo.org/proj/${PN}.git/snapshot/${P}.tar.gz"

LICENSE="GPL-2"
SLOT="0"
KEYWORDS="*"

RDEPEND="app-admin/eselect"

src_prepare() {
	default
	eautoreconf
}
