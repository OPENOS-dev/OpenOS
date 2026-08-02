EAPI=7

inherit flag-o-matic multilib multilib-minimal cros-toolchain-funcs

DESCRIPTION="Complete solution to record/convert/stream audio and video. Includes libavcodec"
HOMEPAGE="https://ffmpeg.org/"
SRC_URI="mirror://gentoo/${P}.tar.xz"

SLOT="0"
LICENSE="LGPL-2.1"
KEYWORDS="*"
BDEPEND="dev-lang/nasm"

PATCHES=(
	"${FILESDIR}/ffmpeg-dont-omit-frame-pointers.patch"
)

multilib_src_configure() {
	local conf=(
		--disable-all
		--disable-autodetect
		--disable-static
		--enable-shared
		--enable-avcodec
		--enable-avutil
		--enable-encoder=aac
		--enable-cross-compile
		--arch=$(tc-arch-kernel)
		--cross-prefix=${CHOST}-
		--host-cc="$(tc-getBUILD_CC)"
		--target-os=linux
	)

	set -- "${S}/configure" \
		--prefix="${EPREFIX}/usr" \
		--shlibdir="${EPREFIX}/usr/$(get_libdir)" \
		--cc="$(tc-getCC)" \
		--cxx="$(tc-getCXX)" \
		--ar="$(tc-getAR)" \
		--nm="$(tc-getNM)" \
		--strip="$(tc-getSTRIP)" \
		--ranlib="$(tc-getRANLIB)" \
		--pkg-config="$(tc-getPKG_CONFIG)" \
		--optflags="${CFLAGS}" \
		--disable-stripping \
		"${conf[@]}"
	echo "${@}"
	"${@}" || die
}

multilib_src_install() {
	emake V=1 DESTDIR="${D}" install
}

multilib_src_install_all() {
	dodoc Changelog README.md CREDITS doc/*.txt doc/APIchanges
	[ -f "RELEASE_NOTES" ] && dodoc "RELEASE_NOTES"
}
