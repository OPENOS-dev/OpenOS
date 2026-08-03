# Copyright 1999-2022 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

DIST_AUTHOR=OALDERS
DIST_VERSION=6.05
inherit perl-module

DESCRIPTION="Date conversion for HTTP date formats"

SLOT="0"
KEYWORDS="*"

RDEPEND="
	!<dev-perl/libwww-perl-6
	virtual/perl-Exporter
	>=virtual/perl-Time-Local-1.280.0
	dev-perl/TimeDate
"
BDEPEND="${RDEPEND}
	virtual/perl-ExtUtils-MakeMaker
"
