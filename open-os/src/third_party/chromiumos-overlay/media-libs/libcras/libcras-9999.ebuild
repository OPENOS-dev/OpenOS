# Copyright 2024 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

EAPI=7

PYTHON_COMPAT=( python3_11 )

CROS_WORKON_INCREMENTAL_BUILD=1
CROS_WORKON_PROJECT=(
	"chromiumos/third_party/adhd"
	"chromiumos/third_party/webrtc-apm"
)
CROS_WORKON_LOCALNAME=(
	"adhd"
	"webrtc-apm"
)
CROS_WORKON_SUBTREE=(
	""
	""
)
CROS_WORKON_DESTDIR=(
	"${S}/adhd"
	"${S}/webrtc-apm"
)
CROS_WORKON_USE_VCSID=1

inherit python-any-r1 cros-toolchain-funcs cros-bazel cros-fuzzer cros-sanitizers cros-workon
inherit cros-debug systemd user cros-protobuf
inherit adhd
# edit this line for adhd.eclass changes.

DESCRIPTION="Google A/V Daemon; client library only"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/third_party/adhd/"

LICENSE="Apache-2.0 BSD-Google MIT"
KEYWORDS="~*"

COMMON_DEPEND="
	>=media-libs/alsa-lib-1.1.6-r3:=
"

RDEPEND="
	${COMMON_DEPEND}
	!<media-sound/adhd-0.0.8
"

# chromeos-chrome links against `libcras`. libcras only needs `adhd` when
# it's actually executing on the system. It's not needed as a transitive
# dependency.
PDEPEND="media-sound/adhd"

DEPEND="
	${COMMON_DEPEND}
	dev-cpp/gtest
	dev-libs/libpthread-stubs:=
"

BDEPEND="
	chromeos-base/minijail
	sys-apps/which
	sys-devel/gettext
	${PYTHON_DEPS}
"

src_configure() {
	cros_optimize_package_for_speed
	adhd_src_configure
}

src_compile() {
	rm -f "${T}/media_libs_libcras.tar"

	adhd_build_tar media_libs_libcras.tar
}

src_install() {
	cd "${S}/adhd" || die

	einfo Installing media_libs_libcras.tar
	tar -C "${D}" -xf "${T}/media_libs_libcras.tar" || die
}
