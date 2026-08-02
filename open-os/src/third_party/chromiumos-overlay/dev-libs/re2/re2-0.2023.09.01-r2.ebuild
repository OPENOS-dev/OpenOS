# Copyright 2012-2023 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

inherit cros-sanitizers multilib-minimal cros-toolchain-funcs

# Different date format used upstream.
RE2_VER=${PV#0.}
RE2_VER=${RE2_VER//./-}

DESCRIPTION="An efficient, principled regular expression library"
HOMEPAGE="https://github.com/google/re2"
SRC_URI="https://github.com/google/re2/archive/${RE2_VER}.tar.gz -> re2-${RE2_VER}.tar.gz"

LICENSE="BSD"
# NOTE: Always run libre2 through abi-compliance-checker!
# https://abi-laboratory.pro/tracker/timeline/re2/
SONAME="11"
SLOT="0/${SONAME}"
KEYWORDS="*"
IUSE="icu"

BDEPEND="icu? ( virtual/pkgconfig )"
DEPEND="
	icu? ( dev-libs/icu:0=[${MULTILIB_USEDEP}] )
	dev-cpp/abseil-cpp:=
"
RDEPEND="${DEPEND}"

S="${WORKDIR}/re2-${RE2_VER}"

DOCS=( AUTHORS CONTRIBUTORS README doc/syntax.txt )
HTML_DOCS=( doc/syntax.html )

src_prepare() {
	default
	grep -q "^SONAME=${SONAME}\$" Makefile || die "SONAME mismatch"
	if use icu; then
		sed -i -e 's:^# \(\(CC\|LD\)ICU=.*\):\1:' Makefile || die
	fi
	# TODO(https://code-review.git.corp.google.com/c/re2/+/61950)
	# Remove after this upgrading to a version with the linked change.
	sed -i -e 's:shell pkg-config:shell $(PKG_CONFIG):g' Makefile || die
	multilib_copy_sources
}

src_configure() {
	# ChromeOS: Prevent exporting inline symbols to improve startup speed.
	#           (go/cros-symbol-slimming)
	append-cxxflags -fvisibility-inlines-hidden
	# ChromeOS: Assume no interposition and pre-bind DSO-local symbols to
	#           improve startup speed. (go/cros-symbol-slimming)
	append-ldflags -Wl,-Bsymbolic-non-weak

	tc-export AR CXX PKG_CONFIG
	sanitizers-setup-env
}

multilib_src_compile() {
	emake SONAME="${SONAME}" shared
}

multilib_src_install() {
	emake SONAME="${SONAME}" DESTDIR="${D}" prefix="${EPREFIX}/usr" libdir="\$(exec_prefix)/$(get_libdir)" shared-install
}
