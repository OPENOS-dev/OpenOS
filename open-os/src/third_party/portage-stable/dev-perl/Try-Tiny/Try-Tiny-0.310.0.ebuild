# Copyright 1999-2022 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

DIST_AUTHOR=ETHER
DIST_VERSION=0.31
inherit perl-module

DESCRIPTION="Minimal try/catch with proper localization of \$@"

LICENSE="MIT"
SLOT="0"
KEYWORDS="*"
IUSE="minimal"

RDEPEND="
	!<=dev-perl/Try-Tiny-Except-0.10.0
	!minimal? (
		|| ( >=virtual/perl-Scalar-List-Utils-1.400.0 dev-perl/Sub-Name )
	)
	virtual/perl-Carp
	>=virtual/perl-Exporter-5.570.0
"
BDEPEND="${RDEPEND}
	virtual/perl-ExtUtils-MakeMaker
"
