# Copyright 1999-2022 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

DIST_AUTHOR=CHRISN
DIST_VERSION=1.92
DIST_EXAMPLES=("examples/*")
inherit perl-module

DESCRIPTION="Perl extension for using OpenSSL"

LICENSE="Artistic-2"
SLOT="0"
KEYWORDS="*"
IUSE="minimal examples"

RDEPEND="
	dev-libs/openssl:=
	virtual/perl-MIME-Base64
"
DEPEND="${RDEPEND}"
BDEPEND="${RDEPEND}
	virtual/perl-ExtUtils-MakeMaker
	virtual/perl-File-Spec
"

PATCHES=(
	"${FILESDIR}/${PN}-1.88-fix-network-tests.patch"
)

PERL_RM_FILES=(
	# Hateful author tests
	't/local/01_pod.t'
	't/local/02_pod_coverage.t'
	't/local/kwalitee.t'
)

src_configure() {
	export NETWORK_TESTS=no
	export LIBDIR=$(get_libdir)
	export OPENSSL_PREFIX="${ESYSROOT}/usr"
	perl-module_src_configure
}

src_compile() {
	mymake=(
		OPTIMIZE="${CFLAGS}"
		OPENSSL_PREFIX="${ESYSROOT}"/usr
	)
	perl-module_src_compile
}
