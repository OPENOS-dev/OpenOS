# Copyright 1999-2023 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

DESCRIPTION="Virtual for ${PN#perl-}"
SLOT="0"
KEYWORDS="*"
LICENSE="metapackage"

RDEPEND="
	|| ( =dev-lang/perl-5.42* ~perl-core/${PN#perl-}-${PV} )
	!<perl-core/${PN#perl-}-${PV}
	!>perl-core/${PN#perl-}-${PV}-r999
"
