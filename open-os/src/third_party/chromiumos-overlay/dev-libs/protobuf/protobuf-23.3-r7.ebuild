# Copyright 2008-2023 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

inherit cros-sanitizers cmake-multilib elisp-common flag-o-matic cros-toolchain-funcs

if [[ "${PV}" == *9999 ]]; then
	inherit git-r3

	EGIT_REPO_URI="https://github.com/protocolbuffers/protobuf.git"
	EGIT_SUBMODULES=()
else
	SRC_URI="https://github.com/protocolbuffers/protobuf/archive/v${PV}.tar.gz -> ${P}.tar.gz"
	KEYWORDS="*"
fi

DESCRIPTION="Google's Protocol Buffers - Extensible mechanism for serializing structured data"
HOMEPAGE="https://protobuf.dev/"

LICENSE="BSD"
# The SLOT's patch version is bumped to .2 to signal an ABI change. This is
# critical for cases such as the recent and future abseil-cpp upgrade. The new
# abseil is not a drop-in replacement for the old one. By changing the sub-SLOT,
# we force Portage to rebuild all packages that depend on protobuf. This ensures
# they are re-linked against the new abseil, preventing runtime crashes from ABI
# incompatibility.
SLOT="0/$(ver_cut 1-2).2"
IUSE="emacs examples system-protoc test zlib"
RESTRICT="!test? ( test )"

BDEPEND="emacs? ( app-editors/emacs:* )"
COMMON_DEPEND="
	>=dev-cpp/abseil-cpp-20240116.3:=[${MULTILIB_USEDEP}]
	zlib? ( sys-libs/zlib[${MULTILIB_USEDEP}] )
"
DEPEND="
	${COMMON_DEPEND}
	test? ( >=dev-cpp/gtest-1.9[${MULTILIB_USEDEP}] )
"
RDEPEND="
	${COMMON_DEPEND}
	emacs? ( app-editors/emacs:* )
"

PATCHES=(
	"${FILESDIR}/${PN}-23.3-disable-32-bit-tests.patch"
	"${FILESDIR}/${PN}-23.3-static_assert-failure.patch"
	"${FILESDIR}/${PN}-23.3-export-mergefromimpl.patch"
	"${FILESDIR}/${PN}-23.3-absl-fix.patch"
)

DOCS=( CONTRIBUTORS.txt README.md )

src_configure() {
	# ChromeOS: enable large file support.
	# Upstream bug: https://bugs.gentoo.org/896086
	append-lfs-flags
	sanitizers-setup-env
	# ChromeOS: Prevent exporting inline symbols to improve startup speed.
	#           (go/cros-symbol-slimming)
	append-cxxflags -fvisibility-inlines-hidden
	# ChromeOS: Assume no interposition and pre-bind DSO-local symbols to
	#           improve startup speed. (go/cros-symbol-slimming)
	append-ldflags -Wl,-Bsymbolic-non-weak

	if tc-ld-is-gold; then
		# https://sourceware.org/PR24527
		tc-ld-disable-gold
	fi

	cmake-multilib_src_configure
}

multilib_src_configure() {
	local mycmakeargs=(
		-DCMAKE_CXX_STANDARD=20  # We used gnu++20 on ChromeOS.
		-Dprotobuf_DISABLE_RTTI=ON
		-Dprotobuf_BUILD_EXAMPLES=$(usex examples)
		-Dprotobuf_WITH_ZLIB=$(usex zlib)
		-Dprotobuf_BUILD_TESTS=$(usex test)
		-Dprotobuf_BUILD_PROTOC_BINARIES=$(usex system-protoc OFF ON)
		-Dprotobuf_BUILD_LIBPROTOC=$(usex system-protoc OFF ON)
		-Dprotobuf_USE_EXTERNAL_GTEST=ON
		-Dprotobuf_ABSL_PROVIDER=package
	)
	if tc-is-cross-compiler || use system-protoc; then
		mycmakeargs+=(
			-DWITH_PROTOC=protoc
		)
	else
		mycmakeargs+=(
			-DWITH_PROTOC=0
		)
	fi

	cmake_src_configure
}

src_compile() {
	cmake-multilib_src_compile

	if use emacs; then
		elisp-compile editors/protobuf-mode.el
	fi
}

multilib_src_install_all() {
	find "${ED}" -name "*.la" -delete || die

	# Sanity check to ensure the main shared library was built correctly.
	# This check was made more flexible for the abseil upgrade. The sub-SLOT was
	# bumped to 23.3.2, as shown around line 28 above, SLOT="0/$(ver_cut 1-2).2",
	# to signal an ABI change, but the upstream source still builds a library
	# with version 23.3.0. This glob (*) allows the build
	# to find the correct library without failing on the version mismatch.
	if ! compgen -G "${ED}/usr/$(get_libdir)/libprotobuf.so.${PV}.*" > /dev/null; then
		eerror "No matching library found for version ${PV}.*\n" \
			"Expected a file like: ${ED}/usr/$(get_libdir)/libprotobuf.so.${PV}.patch"
		die "Protobuf library not found."
	fi

	insinto /usr/share/vim/vimfiles/syntax
	doins editors/proto.vim
	insinto /usr/share/vim/vimfiles/ftdetect
	doins "${FILESDIR}/proto.vim"

	if use emacs; then
		elisp-install ${PN} editors/protobuf-mode.el*
		elisp-site-file-install "${FILESDIR}/70${PN}-gentoo.el"
	fi

	if use examples; then
		DOCS+=(examples)
		docompress -x /usr/share/doc/${PF}/examples
	fi

	# The build generates both libprotobuf.so (full) and libprotobuf-lite.so.
	# Some packages may incorrectly link against both, leading to symbol
	# conflicts. To prevent this, we remove the lite version and replace it
	# with a symlink to the full library.
	# NOTE: The version is hardcoded to 23.3.0 because that is the literal
	# filename produced by the upstream build. This is different from the
	# sub-SLOT (bumped to 23.3.2 for the abseil upgrade), which forces rebuilds.
	rm "${ED}/usr/$(get_libdir)/libprotobuf-lite.so.23.3.0" || die
	dosym ./libprotobuf.so.23.3.0 "/usr/$(get_libdir)/libprotobuf-lite.so.23.3.0"

	einstalldocs
}

pkg_postinst() {
	use emacs && elisp-site-regen
}

pkg_postrm() {
	use emacs && elisp-site-regen
}
