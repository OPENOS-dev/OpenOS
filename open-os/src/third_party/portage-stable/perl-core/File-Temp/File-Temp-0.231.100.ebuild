# Copyright 1999-2022 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

DIST_AUTHOR=ETHER
DIST_VERSION=0.2311
inherit perl-module

DESCRIPTION="File::Temp can be used to create and open temporary files in a safe way"

SLOT="0"
KEYWORDS="*"
IUSE=""

# bug 390719
PATCHES=( "${FILESDIR}/${PN}-0.230.0-symlink-safety.patch" )
