# Copyright 1999-2023 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

DIST_AUTHOR=LEONT
DIST_VERSION=0.010
inherit perl-module

DESCRIPTION="Fast and correct UTF-8 IO"

SLOT="0"
KEYWORDS="*"
IUSE="test"

RDEPEND="
	virtual/perl-XSLoader
"
BDEPEND="
	${RDEPEND}
	virtual/perl-ExtUtils-MakeMaker
	test? (
		virtual/perl-Carp
		virtual/perl-Exporter
		virtual/perl-File-Spec
		virtual/perl-IO
		dev-perl/Test-Exception
		>=virtual/perl-Test-Simple-0.880.0
	)
"
