# Copyright 1999-2020 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_WORKON_COMMIT="35743d94643ad7337bd220519a800f00e95ad92f"
CROS_WORKON_TREE="ede0d81ab17b9a8f5be54a057962e383567d58c9"
CROS_WORKON_PROJECT="chromiumos/third_party/libnih"
CROS_WORKON_LOCALNAME="../third_party/libnih"

inherit autotools cros-sanitizers cros-workon cros-toolchain-funcs multilib flag-o-matic usr-ldscript

DESCRIPTION="Light-weight 'standard library' of C functions"
HOMEPAGE="https://launchpad.net/libnih"

LICENSE="GPL-2"
KEYWORDS="*"
IUSE="+dbus nls static-libs +threads"
# TODO(b/341759381): libnih has tests, but they fail in some cases on CI.
# Re-enable them once we figure that out.
RESTRICT="
	!x86? ( !amd64? ( test ) )
	test
"

# The configure phase will check for valgrind headers, and the tests will use
# that header, but only to do dynamic valgrind detection.  The tests aren't
# run directly through valgrind, only by developers directly.  So don't bother
# depending on valgrind here. #559830
RDEPEND="dbus? ( dev-libs/expat >=sys-apps/dbus-1.2.16 )"
DEPEND="${RDEPEND}"
BDEPEND="
	sys-devel/gettext
	virtual/pkgconfig"

src_prepare() {
	default
	eautoreconf
}

src_configure() {
	append-lfs-flags
	sanitizers-setup-env
	# libnih destructors seem to confuse the stack-use-after-scope
	# sanitizer.
	append-cflags -fno-sanitize-address-use-after-scope

	# libnih also has _many_ places where it calls functions through incorrect
	# pointers. These are functionally pedantic, e.g., destructors are
	# technically `void (*)(void *)`, but the destructor for `Foo`'s signature is
	# `void (*)(Foo *)`.
	append-cflags -fno-sanitize=function

	econf \
		$(use_with dbus) \
		$(use_enable nls) \
		$(use_enable static-libs static) \
		$(use_enable threads) \
		$(use_enable threads threading)
}

src_install() {
	default

	# we need to be in / because upstart needs libnih
	gen_usr_ldscript -a nih "$(use dbus && echo nih-dbus)"
	use static-libs || rm -f "${ED}/usr/$(get_libdir)/*.la"
}

src_test() {
	# libnih tests are currently very leaky.
	export ASAN_OPTIONS+=":detect_leaks=0"
	default
}
