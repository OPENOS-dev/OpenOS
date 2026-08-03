# Copyright 1999-2023 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

DIST_AUTHOR=RJBS
DIST_VERSION=0.990
inherit perl-module

DESCRIPTION="Sophisticated exporter for custom-built routines"

SLOT="0"
KEYWORDS="*"

RDEPEND="
	virtual/perl-Carp
	>=dev-perl/Data-OptList-0.100.0
	>=dev-perl/Params-Util-0.140.0
	>=dev-perl/Sub-Install-0.920.0
"
BDEPEND="${RDEPEND}"
