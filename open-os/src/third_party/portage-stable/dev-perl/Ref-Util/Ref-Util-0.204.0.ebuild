# Copyright 1999-2022 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

DIST_AUTHOR=ARC
DIST_VERSION=0.204
inherit perl-module

DESCRIPTION="Utility functions for checking references"
SLOT="0"
KEYWORDS="*"
LICENSE="MIT"
IUSE="test +xs"

RDEPEND="
	xs? (
		dev-perl/Ref-Util-XS
	)
	>=virtual/perl-Exporter-5.570.0
"
BDEPEND="${RDEPEND}
	virtual/perl-ExtUtils-MakeMaker
	virtual/perl-Text-ParseWords
	test? (
		>=virtual/perl-CPAN-Meta-2.120.900
		>=virtual/perl-Test-Simple-0.960.0
		virtual/perl-File-Spec
	)
"
