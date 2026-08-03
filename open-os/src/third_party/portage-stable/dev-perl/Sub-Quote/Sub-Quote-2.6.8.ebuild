# Copyright 1999-2023 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

DIST_AUTHOR=HAARG
DIST_VERSION=2.006008
inherit perl-module

DESCRIPTION="Efficient generation of subroutines via string eval"
SLOT="0"
KEYWORDS="*"
IUSE="minimal test"

RDEPEND="
	!<dev-perl/Moo-2.3.0
	virtual/perl-Scalar-List-Utils
"
BDEPEND="
	${RDEPEND}
	virtual/perl-ExtUtils-MakeMaker
	test? (
		>=virtual/perl-Test-Simple-0.940.0
	)
"
