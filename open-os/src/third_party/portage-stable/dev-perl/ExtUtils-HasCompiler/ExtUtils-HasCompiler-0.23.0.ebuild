# Copyright 1999-2022 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7
DIST_AUTHOR=LEONT
DIST_VERSION=0.023

inherit perl-module

DESCRIPTION="Check for the presence of a compiler"
SLOT="0"
KEYWORDS="*"

RDEPEND="
	virtual/perl-Carp
	virtual/perl-Exporter
	virtual/perl-File-Spec
	virtual/perl-File-Temp
"
BDEPEND="${RDEPEND}
"
