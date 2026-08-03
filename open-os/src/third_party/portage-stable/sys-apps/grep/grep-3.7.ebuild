# Copyright 1999-2021 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

inherit flag-o-matic toolchain-funcs

DESCRIPTION="GNU regular expression matcher"
HOMEPAGE="https://www.gnu.org/software/grep/"
SRC_URI="mirror://gnu/${PN}/${P}.tar.xz"

LICENSE="GPL-3+"
SLOT="0"
KEYWORDS="*"
IUSE="nls pcre static"

# We lack dev-libs/libsigsegv[static-libs] for now
REQUIRED_USE="static? ( !sparc )"

LIB_DEPEND="pcre? ( >=dev-libs/libpcre-7.8-r1[static-libs(+)] )
	sparc? ( dev-libs/libsigsegv )"
RDEPEND="!static? ( ${LIB_DEPEND//\[static-libs(+)]} )
	nls? ( virtual/libintl )
	virtual/libiconv"
DEPEND="${RDEPEND}
	static? ( ${LIB_DEPEND} )"
BDEPEND="
	virtual/pkgconfig
	nls? ( sys-devel/gettext )
"

DOCS=( AUTHORS ChangeLog NEWS README THANKS TODO )

src_prepare() {
	sed -i \
		-e "s:@SHELL@:${EPREFIX}/bin/sh:g" \
		-e "s:@grep@:${EPREFIX}/bin/grep:" \
		src/egrep.sh || die #523898

	default
}

src_configure() {
	use static && append-ldflags -static

	# We used to turn this off unconditionally (bug #673524) but we now
	# allow it for cases where libsigsegv is better for userspace handling
	# of stack overflows.
	# In particular, it's necessary for sparc: bug #768135
	export ac_cv_libsigsegv=$(usex sparc)

	# Always use pkg-config to get lib info for pcre.
	export ac_cv_search_pcre_compile=$(
		usex pcre "$($(tc-getPKG_CONFIG) --libs $(usex static --static '') libpcre)" ''
	)
	local myeconfargs=(
		--bindir="${EPREFIX}"/bin
		$(use_enable nls)
		$(use_enable pcre perl-regexp)
	)
	econf "${myeconfargs[@]}"
}
