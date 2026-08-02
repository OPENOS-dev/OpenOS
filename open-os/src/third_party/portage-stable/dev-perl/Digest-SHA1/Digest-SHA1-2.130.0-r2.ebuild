# Copyright 1999-2021 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

DIST_AUTHOR=GAAS
DIST_VERSION=2.13
inherit perl-module

DESCRIPTION="NIST SHA message digest algorithm"

SLOT="0"
KEYWORDS="*"

RDEPEND="
	>=virtual/perl-Digest-1.0.0
"
BDEPEND="${DEPEND}
	virtual/perl-ExtUtils-MakeMaker
"

src_compile() {
	mymake=(
		"OPTIMIZE=${CFLAGS}"
	)
	perl-module_src_compile
}
