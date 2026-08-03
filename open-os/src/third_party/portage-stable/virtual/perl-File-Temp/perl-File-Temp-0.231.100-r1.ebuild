# Copyright 1999-2022 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

DESCRIPTION="Virtual for ${PN#perl-}"
SLOT="0"
KEYWORDS="*"
LICENSE="metapackage"

RDEPEND="
	|| ( =dev-lang/perl-5.42* ~perl-core/${PN#perl-}-${PV} )
"

# this is the dev-lang/perl-5.34 and dev-lang/perl-5.36 version but we need the security patch
