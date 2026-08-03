# Copyright 1999-2022 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

DIST_AUTHOR=ISHIGAKI
DIST_VERSION=4.03
DIST_EXAMPLES=("eg/*")
inherit perl-module

DESCRIPTION="JSON (JavaScript Object Notation) encoder/decoder"

SLOT="0"
KEYWORDS="*"
IUSE="test +xs"
RESTRICT="!test? ( test )"

RDEPEND="xs? ( >=dev-perl/JSON-XS-2.340.0 )"
BDEPEND="
	virtual/perl-ExtUtils-MakeMaker
	test? ( virtual/perl-Test-Simple )
"

src_test() {
	perl_rm_files t/00_pod.t
	perl-module_src_test
}
