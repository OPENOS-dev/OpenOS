# Copyright 1999-2023 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

DESCRIPTION="Virtual for ${PN#perl-}"
SLOT="0"
KEYWORDS="*"

RDEPEND="
	|| ( =dev-lang/perl-5.42* =dev-lang/perl-5.38* ~perl-core/${PN#perl-}-${PV} )
	!<perl-core/${PN#perl-}-${PV}
	!>perl-core/${PN#perl-}-${PV}-r999
	>=virtual/perl-Compress-Raw-Bzip2-2.204.1_rc
	>=virtual/perl-Compress-Raw-Zlib-2.204.1_rc
"
# Dependencies on Compress-Raw* must be kept in step
# but sometimes not .... use ${PV} when you can.
