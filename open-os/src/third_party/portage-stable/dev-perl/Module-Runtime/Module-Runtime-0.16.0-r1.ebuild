# Copyright 1999-2022 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

DIST_AUTHOR=ZEFRAM
DIST_VERSION=0.016
inherit perl-module

DESCRIPTION="Runtime module handling"
SLOT="0"
KEYWORDS="*"
IUSE="test"

RDEPEND=""
BDEPEND="${RDEPEND}
	dev-perl/Module-Build
	test? (
		virtual/perl-Test-Simple
	)
"
DEPEND="dev-perl/Module-Build"

src_test() {
	perl_rm_files t/pod_cvg.t t/pod_syn.t
	perl-module_src_test
}
