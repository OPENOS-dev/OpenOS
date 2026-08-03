# Copyright 1999-2021 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

DIST_AUTHOR=CAPOEIRAB
DIST_VERSION=9999.32
inherit perl-module

DESCRIPTION="Simple and Efficient Reading/Writing/Modifying of Complete Files"

SLOT="0"
KEYWORDS="*"
IUSE="test"
RESTRICT="!test? ( test )"

RDEPEND="
	virtual/perl-Carp
	>=virtual/perl-Exporter-5.570.0
	>=virtual/perl-File-Spec-3.10.0
	virtual/perl-File-Temp
	virtual/perl-IO
"
BDEPEND="${RDEPEND}
	virtual/perl-ExtUtils-MakeMaker
"

mydoc="extras/slurp_article.pod"
