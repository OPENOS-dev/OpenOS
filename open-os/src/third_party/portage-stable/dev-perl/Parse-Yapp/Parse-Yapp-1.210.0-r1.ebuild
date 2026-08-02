# Copyright 1999-2022 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

DIST_AUTHOR=WBRASWELL
DIST_VERSION=1.21
DIST_EXAMPLES=( "Calc.yp" "YappParse.yp" )
inherit perl-module

DESCRIPTION="Compiles yacc-like LALR grammars to generate Perl OO parser modules"

SLOT="0"
KEYWORDS="*"

RDEPEND="
	!<net-fs/samba-4.10.6
"

BDEPEND="${RDEPEND}
	virtual/perl-ExtUtils-MakeMaker
"

mydoc="docs/*"
