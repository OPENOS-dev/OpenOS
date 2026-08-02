# Copyright 1999-2023 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

DIST_AUTHOR=LEONT
DIST_VERSION=0.014
DIST_EXAMPLES=( "bench/*" )
inherit perl-module

DESCRIPTION="A simple, sane and efficient module to slurp a file"

SLOT="0"
KEYWORDS="*"
IUSE="test"

RDEPEND="
	dev-perl/PerlIO-utf8_strict
	virtual/perl-Carp
	>=virtual/perl-Encode-2.110.0
	>=virtual/perl-Exporter-5.570.0
"
BDEPEND="${RDEPEND}
	virtual/perl-ExtUtils-MakeMaker
	test? (
		virtual/perl-File-Spec
		virtual/perl-File-Temp
		virtual/perl-Test-Simple
		dev-perl/Test-Warnings
	)
"
