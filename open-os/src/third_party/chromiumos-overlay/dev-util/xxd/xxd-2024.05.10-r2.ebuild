# Copyright 1999-2010 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header: $

EAPI="7"

inherit flag-o-matic cros-toolchain-funcs

DESCRIPTION="make a hexdump or do the reverse"
HOMEPAGE="https://github.com/vim/vim/tree/master/src/xxd"
SRC_URI="https://raw.githubusercontent.com/vim/vim/v9.1.0608/src/xxd/xxd.c -> xxd-2024-05-10.c"

LICENSE="GPL-2"
SLOT="0"
KEYWORDS="*"
IUSE=""

src_unpack() {
	mkdir -p "${S}"
	cp "${DISTDIR}/${A}" "${S}/xxd.c"
}

src_prepare() {
	default
	# use implicit make rules as they're better than the makefile
	echo 'all: xxd' > Makefile
}

src_configure() {
	tc-export CC

	# Enable LFS (Large File Support): https://issuetracker.google.com/201531268
	append-lfs-flags
}

src_install() {
	# Has to be /bin rather than /usr/bin due to conflict with vim
	into /
	dobin xxd
}
