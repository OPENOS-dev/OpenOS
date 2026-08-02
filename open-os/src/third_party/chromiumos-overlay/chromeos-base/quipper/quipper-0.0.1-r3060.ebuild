# Copyright 2014 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

# Don't use Makefile.external here as it fetches from the network.
EAPI=7

CROS_WORKON_COMMIT="d57fba8f85abc0d29979909ae7685f2f262518dd"
CROS_WORKON_TREE=("518b50f8b6d01e95cbd933487ed7c6452ac4acb3" "f91b6afd5f2ae04ee9a2c19109a3a4a36f7659e6")
CROS_WORKON_INCREMENTAL_BUILD=1

CROS_WORKON_LOCALNAME="platform2"
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_DESTDIR="${S}/platform2"
# chromiumos-wide-profiling directory is in $SRC_URI, not in platform2.
CROS_WORKON_SUBTREE="common-mk .gn"

PLATFORM_SUBDIR="chromiumos-wide-profiling"

inherit cros-workon platform cros-protobuf

DESCRIPTION="quipper: chromiumos wide profiling"
HOMEPAGE="http://www.chromium.org/chromium-os/profiling-in-chromeos"

GIT_SHA1="42b687e95926099f791847789cc52925ef385ddb"
SRC="quipper-${GIT_SHA1}.tar.gz"
SRC_URI="gs://chromeos-localmirror/distfiles/${SRC}"
SRC_DIR="src/${PN}"

LICENSE="BSD-Google"
KEYWORDS="*"
IUSE=""

COMMON_DEPEND="
	>=dev-cpp/gflags-2.0:=
	>=dev-libs/glib-2.30:=
	dev-libs/openssl:=
	dev-libs/re2:=
	dev-util/perf:=
	virtual/libelf:=
"

RDEPEND="${COMMON_DEPEND}"

DEPEND="${COMMON_DEPEND}
	chromeos-base/protofiles:=
	test? ( app-shells/dash )
"

src_unpack() {
	platform_src_unpack
	mkdir "${S}"

	pushd "${S}" >/dev/null || die
	unpack ${SRC}
	mv "${SRC_DIR}"/{.[!.],}* ./ || die
	eapply "${FILESDIR}"/quipper-disable-flaky-tests.patch
	eapply "${FILESDIR}"/quipper-check-header.patch
	eapply "${FILESDIR}"/quipper-inject-timeout.patch
	eapply "${FILESDIR}"/quipper-platformthreadid.patch
	popd >/dev/null || die
}

src_configure() {
	# Temporarily disable implicit-fallthrough warning.
	# TODO(b/320692200): Remove this after removing the libchrome patch.
	append-cxxflags -Wno-implicit-fallthrough

	platform_src_configure
}

src_compile() {
	# ARM tests run on qemu which is much slower. Exclude some large test
	# data files for non-x86 boards.
	if use x86 || use amd64 ; then
		append-cppflags -DTEST_LARGE_PERF_DATA
	fi

	platform_src_compile
}

src_install() {
	platform_src_install

	dobin "${OUT}"/quipper
}

platform_pkg_test() {
	local tests=(
		integration_tests
		perf_recorder_test
		unit_tests
	)
	for test_bin in "${tests[@]}"; do
		platform_test "run" "${OUT}/${test_bin}" "1"
	done
}
