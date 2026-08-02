# Copyright 1999-2022 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

DIST_AUTHOR=CJM
DIST_VERSION=1.004
inherit perl-module

DESCRIPTION="Open an HTML file with automatic charset detection"

SLOT="0"
KEYWORDS="*"

RDEPEND="
	virtual/perl-Carp
	>=virtual/perl-Encode-2.100.0
	>=virtual/perl-Exporter-5.570.0
"
BDEPEND="${RDEPEND}
	>=virtual/perl-ExtUtils-MakeMaker-6.300.0
"
