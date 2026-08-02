# Copyright 1999-2022 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

DIST_AUTHOR=GRANTM
DIST_VERSION=2.25
inherit perl-module

DESCRIPTION="An API for simple XML files"

SLOT="0"
KEYWORDS="*"

RDEPEND="
	virtual/perl-Storable
	>=dev-perl/XML-NamespaceSupport-1.40.0
	>=dev-perl/XML-SAX-0.150.0
	dev-perl/XML-SAX-Expat
"
BDEPEND="${RDEPEND}
	virtual/perl-ExtUtils-MakeMaker
"

PATCHES=("${FILESDIR}/${PN}-2.25-saxtests.patch")

PERL_RM_FILES=("t/author-pod-syntax.t")
