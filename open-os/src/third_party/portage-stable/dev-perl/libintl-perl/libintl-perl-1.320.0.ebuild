# Copyright 1999-2022 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

DIST_AUTHOR=GUIDO
DIST_VERSION=1.32
DIST_EXAMPLES=("sample/*")
inherit perl-module

DESCRIPTION="High-Level Interface to Uniforum Message Translation"
HOMEPAGE="http://guido-flohr.net/projects/libintl-perl https://metacpan.org/release/libintl-perl"

LICENSE="GPL-3"
SLOT="0"
KEYWORDS="*"
IUSE="minimal"

RDEPEND="
	virtual/libintl
	!minimal? (
		dev-perl/File-ShareDir
	)
	virtual/perl-File-Spec
	>=virtual/perl-version-0.770.0
"
DEPEND="${RDEPEND}
"
BDEPEND="${RDEPEND}
	virtual/perl-ExtUtils-MakeMaker
"

PATCHES=( "${FILESDIR}/${PN}-1.280.0-sanity-2.patch" )
