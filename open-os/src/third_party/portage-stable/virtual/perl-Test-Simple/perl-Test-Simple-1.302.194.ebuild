# Copyright 1999-2023 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

DESCRIPTION="Virtual for ${PN#perl-}"
SLOT="0"
KEYWORDS="*"

RDEPEND="
	|| ( =dev-lang/perl-5.42* =dev-lang/perl-5.38* ~perl-core/${PN#perl-}-${PV} )
	!<dev-perl/Test-Tester-0.114.0
	!<dev-perl/Test-use-ok-0.160.0
"
