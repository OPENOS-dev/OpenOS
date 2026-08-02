# Copyright 1999-2022 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

DIST_AUTHOR=MATTN
DIST_VERSION=1.16
inherit perl-module toolchain-funcs

DESCRIPTION="Check that a library is available"

SLOT="0"
KEYWORDS="*"

RDEPEND="
	virtual/perl-Exporter
	virtual/perl-File-Spec
	>=virtual/perl-File-Temp-0.160.0
"
BDEPEND="${RDEPEND}
	virtual/perl-ExtUtils-MakeMaker
"
PATCHES=(
	"${FILESDIR}/${PN}-1.14-test-toolchain.patch"
)

src_test() {
	unset LD
	[[ -n "${CCLD}" ]] && export LD="${CCLD}"
	tc-export AR RANLIB
	perl-module_src_test
}
