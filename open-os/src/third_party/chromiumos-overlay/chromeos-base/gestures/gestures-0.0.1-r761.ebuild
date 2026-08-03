# Copyright 2012 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7
CROS_WORKON_COMMIT="2fa2b8b73e26733d92f558e3732997549cf3abc0"
CROS_WORKON_TREE="ab5661f6db035db1ecb1e8be924aea789430ae60"
CROS_WORKON_PROJECT="chromiumos/platform/gestures"
CROS_WORKON_LOCALNAME="platform/gestures"
CROS_WORKON_USE_VCSID=1

# TODO(b/380266764): Remove this exemption once the tests are fixed.
# shellcheck disable=SC2034  # eclass variable.
CROS_SANITIZER_DISABLE_LIBCXX_HARDENING=1

inherit cros-toolchain-funcs cros-debug cros-sanitizers cros-workon

DESCRIPTION="Gesture recognizer library"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform/gestures/"
SRC_URI=""

LICENSE="BSD-Google"
KEYWORDS="*"
IUSE="-asan +X"

RDEPEND="chromeos-base/gestures-conf:=
	chromeos-base/libevdev:=
	dev-libs/jsoncpp:=
	virtual/udev"
DEPEND="dev-cpp/gtest:=
	X? ( x11-libs/libXi:= )
	${RDEPEND}"

RESTRICT="!x86? ( !amd64? ( test ) )"

# The last dir must be named "gestures" for include path reasons.
S="${WORKDIR}/gestures"

src_configure() {
	cros_optimize_package_for_speed
	sanitizers-setup-env
	export USE_X11=$(usex X 1 0)
	tc-export CXX PKG_CONFIG
	cros-debug-add-NDEBUG
	default
}

src_compile() {
	emake clean  # TODO(adlr): remove when a better solution exists
	emake
}

src_test() {
	SANITIZE_GESTURES=yes emake test

	# This is an ugly hack that happens to work, but should not be copied.
	LD_LIBRARY_PATH="${SYSROOT}/usr/$(get_libdir)" \
	./test || die
}

src_install() {
	emake DESTDIR="${D}" LIBDIR="/usr/$(get_libdir)" install
}
