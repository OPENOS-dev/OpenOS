# Copyright 1999-2022 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

DIST_AUTHOR=MLEHMANN
DIST_VERSION=1.01
inherit perl-module

DESCRIPTION="simple data types for common serialisation formats"

SLOT="0"
KEYWORDS="*"

RDEPEND="
	dev-perl/common-sense
"
BDEPEND="${RDEPEND}
	virtual/perl-ExtUtils-MakeMaker
"
