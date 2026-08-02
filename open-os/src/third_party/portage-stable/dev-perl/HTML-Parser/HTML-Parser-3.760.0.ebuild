# Copyright 1999-2022 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

DIST_AUTHOR=OALDERS
DIST_VERSION=3.76
inherit perl-module

DESCRIPTION="Parse HTML documents"

SLOT="0"
KEYWORDS="*"
IUSE="test"
RESTRICT="!test? ( test )"

RDEPEND="
	virtual/perl-Carp
	virtual/perl-Exporter
	dev-perl/HTML-Tagset
	dev-perl/HTTP-Message
	virtual/perl-IO
	dev-perl/URI
	virtual/perl-XSLoader
"
BDEPEND="${RDEPEND}
	virtual/perl-ExtUtils-MakeMaker
"

mydoc="ANNOUNCEMENT"
