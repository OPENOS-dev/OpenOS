# Copyright 1999-2023 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

DIST_AUTHOR=SHAY
DIST_VERSION=1.6

inherit perl-module

DESCRIPTION="Implementation of a Singleton class"

SLOT="0"
KEYWORDS="*"

BDEPEND="
	>=virtual/perl-ExtUtils-MakeMaker-6.640.0
"
