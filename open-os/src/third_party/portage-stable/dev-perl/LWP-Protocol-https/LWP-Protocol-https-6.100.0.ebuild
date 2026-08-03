# Copyright 1999-2022 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

DIST_AUTHOR=OALDERS
DIST_VERSION=6.10
inherit perl-module

DESCRIPTION="Provide https support for LWP::UserAgent"

SLOT="0"
KEYWORDS="*"

RDEPEND="
	app-misc/ca-certificates
	>=dev-perl/IO-Socket-SSL-1.540.0
	>=dev-perl/libwww-perl-6.60.0
	>=dev-perl/Net-HTTP-6
"
DEPEND="${RDEPEND}
	virtual/perl-ExtUtils-MakeMaker
"

PATCHES=(
	"${FILESDIR}"/${PN}-6.70.0-etcsslcerts.patch
	"${FILESDIR}"/${PN}-6.70.0-CVE-2014-3230.patch # note: breaks a test, still needed?
)

PERL_RM_FILES=(
	"t/https_proxy.t" # see above
)
