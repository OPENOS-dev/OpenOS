# Copyright 2012 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7
CROS_WORKON_COMMIT="c24ad8bc3682292d289537dfaa19bf549abfcc15"
CROS_WORKON_TREE="5dc8100d1634f2122ab848b37097d29c4a41fd48"
CROS_WORKON_PROJECT="chromiumos/third_party/daisydog"
CROS_WORKON_OUTOFTREE_BUILD="1"

inherit cros-workon cros-toolchain-funcs user

DESCRIPTION="Simple HW watchdog daemon"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/third_party/daisydog"

LICENSE="GPL-2"
SLOT="0/0"
KEYWORDS="*"
IUSE=""

src_prepare() {
	mkdir -p "$(cros-workon_get_build_dir)"
	default
}

src_configure() {
	tc-export CC
	default
}

_emake() {
	emake -C "$(cros-workon_get_build_dir)" \
		top_srcdir="${S}" -f "${S}"/Makefile "$@"
}

src_compile() {
	_emake
}

src_install() {
	_emake DESTDIR="${D}" install
}

pkg_preinst() {
	enewuser watchdog
	enewgroup watchdog
}
