# Copyright 1999-2023 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

DIST_AUTHOR=LEONT
DIST_VERSION=0.017
inherit perl-module

DESCRIPTION="Verify requirements in a CPAN::Meta object"

SLOT="0"
KEYWORDS="*"
IUSE="test"

RDEPEND="
	>=virtual/perl-CPAN-Meta-2.132.830
	>=virtual/perl-CPAN-Meta-Requirements-2.121.0
	virtual/perl-Exporter
	>=virtual/perl-Module-Metadata-1.0.23
"
BDEPEND="
	${RDEPEND}
	>=virtual/perl-ExtUtils-MakeMaker-6.300.0
	test? (
		virtual/perl-Scalar-List-Utils
		>=virtual/perl-Test-Simple-0.880.0
	)
"
