# Copyright 2023 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_WORKON_COMMIT=("362404e596160add78f63bc42ff2081b91941af5" "e6a2b342eccf5046d951cc2457bd645632ce5370" "7cd39fbb81b085d31550b1cefbcf2e20c34095b5" "9537e373c71c26c5495be60d267dff5eb88b180f" "2e909ccdf779939e5caa5ab52851f38f22037ae9" "ef9626806649d45cd1b5dd692695eae82aff5542")
CROS_WORKON_TREE=("518b50f8b6d01e95cbd933487ed7c6452ac4acb3" "b8033e453c7d9518619e90fb100d7d90d7b4026d" "0c37ed3c9ccea6dabfe1d4fda838289385c54ec0" "8690cf34530625a393e76d599b01742a250bdb7b" "3d01071ecc84cb5a5781cd4a17dc0efe279a6169" "f91b6afd5f2ae04ee9a2c19109a3a4a36f7659e6" "9b2299fe5f7bfb01bfed3cf6466c2f30887fd0a9" "489bf1abd1a3c0300f1eda404af8d19eccedd527" "6fadd8addab8504349cdeefe51b583b97c2ae7f4" "ae1614ebb22b8aa59ecd0d29e1a0e162deaa2d09" "d77790723fe1f66f2ab79eb8e18ea3ac7f432f96")
inherit cros-constants

CROS_WORKON_INCREMENTAL_BUILD="1"
CROS_WORKON_PROJECT=(
	"chromiumos/platform2"
	"aosp/platform/system/keymaster"
	"aosp/platform/system/core/libcutils"
	"aosp/platform/system/libbase"
	"aosp/platform/system/logging"
	"platform/system/libcppbor")

CROS_WORKON_REPO=(
	"${CROS_GIT_HOST_URL}"
	"${CROS_GIT_HOST_URL}"
	"${CROS_GIT_HOST_URL}"
	"${CROS_GIT_HOST_URL}"
	"${CROS_GIT_HOST_URL}"
	"${CROS_GIT_AOSP_URL}"
)

CROS_WORKON_EGIT_BRANCH=(
	"main"
	"android13-platform-release"
	"master"
	"master"
	"master"
	"main"
	)

CROS_WORKON_LOCALNAME=(
	"platform2"
	"aosp/system/keymint-local"
	"aosp/system/core/libcutils"
	"aosp/system/libbase"
	"aosp/system/logging"
	"aosp/system/libcppbor"
)

CROS_WORKON_DESTDIR=(
	"${S}/platform2"
	"${S}/aosp/system/keymint-local"
	"${S}/aosp/system/core/libcutils"
	"${S}/aosp/system/libbase"
	"${S}/aosp/system/logging"
	"${S}/aosp/system/libcppbor"
)

CROS_WORKON_SUBTREE=(
	"common-mk featured arc/keymint libarc-attestation metrics .gn"
	""
	""
	""
	""
	""
)

PLATFORM_SUBDIR="arc/keymint"

# Do not run test parallelly until unit tests are fixed.
# shellcheck disable=SC2034
PLATFORM_PARALLEL_GTEST_TEST="no"

# This BoringSSL integration follows go/boringssl-cros.
# DO NOT COPY TO OTHER PACKAGES WITHOUT CONSULTING SECURITY TEAM.
BORINGSSL_PN="boringssl"
BORINGSSL_PV="3a667d10e94186fd503966f5638e134fe9fb4080"
BORINGSSL_P="${BORINGSSL_PN}-${BORINGSSL_PV}"
BORINGSSL_OUTDIR="${WORKDIR}/boringssl_outputs/"

CMAKE_USE_DIR="${WORKDIR}/${BORINGSSL_P}"
BUILD_DIR="${WORKDIR}/${BORINGSSL_P}_build"

inherit flag-o-matic cmake cros-workon platform cros-protobuf

DESCRIPTION="Android keymint service in Chrome OS."
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform2/+/HEAD/arc/keymint"
SRC_URI="https://github.com/google/${BORINGSSL_PN}/archive/${BORINGSSL_PV}.tar.gz -> ${BORINGSSL_P}.tar.gz"

LICENSE="BSD-Google"
KEYWORDS="*"
IUSE="
	+seccomp
	keymint
"

RDEPEND="
	chromeos-base/chaps:=
	chromeos-base/cryptohome:=
	chromeos-base/cryptohome-client:=
	chromeos-base/featured:=
	chromeos-base/libarc-attestation:=
	chromeos-base/libcrossystem:=
	>=chromeos-base/metrics-0.0.1-r3152:=
	chromeos-base/minijail:=
	dev-libs/re2:=
	acct-group/arc-keymintd
	acct-user/arc-keymintd
"

DEPEND="
	${RDEPEND}
	chromeos-base/session_manager-client:=
	chromeos-base/system_api:=
"

BDEPEND="
	chromeos-base/minijail
	sys-devel/arc-toolchain-t
	dev-lang/perl
"

HEADER_TAINT="#ifdef CHROMEOS_OPENSSL_IS_OPENSSL
#error \"Do not mix OpenSSL and BoringSSL headers.\"
#endif
#define CHROMEOS_OPENSSL_IS_BORINGSSL\n"

src_unpack() {
	platform_src_unpack
	unpack "${BORINGSSL_P}.tar.gz"
	# Taint BoringSSL headers so they don't silently mix with OpenSSL.
	find "${BORINGSSL_P}/include/openssl" -type f -exec awk -i inplace -v \
		"taint=${HEADER_TAINT}" 'NR == 1 {print taint} {print}' {} \;
	(cd "${WORKDIR}/${BORINGSSL_P}" &&
		eapply "${FILESDIR}/boringssl-suppress-unused-but-set-variable.patch" &&
		eapply "${FILESDIR}/boringssl-memchr-c23-const.patch") || die
}

src_prepare() {
	cmake_src_prepare

	# Expose libhardware headers from arc-toolchain-p.
	local arc_arch="${ARCH}"
	# arm needs to use arm64 directory, which provides combined arm/arm64
	# headers.
	if [[ "${ARCH}" == "arm" ]]; then
		arc_arch="arm64"
	fi

	mkdir -p "${WORKDIR}/libhardware/include" || die

	cp -rfp "/opt/android-t/${arc_arch}/usr/include/hardware" "${WORKDIR}/libhardware/include" || die
	cp -rfp "/opt/android-t/${arc_arch}/usr/include/android-base" "${WORKDIR}/libhardware/include" || die
	cp -rfp "/opt/android-t/${arc_arch}/usr/include/cutils" "${WORKDIR}/libhardware/include" || die
	cp -rfp "/opt/android-t/${arc_arch}/usr/include/android" "${WORKDIR}/libhardware/include" || die
	cp -rfp "/opt/android-t/${arc_arch}/usr/include/log" "${WORKDIR}/libhardware/include" || die
	cp -rfp "/opt/android-t/${arc_arch}/usr/include/system" "${WORKDIR}/libhardware/include" || die

	append-cxxflags "-I${WORKDIR}/libhardware/include"

	# Expose BoringSSL headers and outputs.
	append-cxxflags "-I${WORKDIR}/${BORINGSSL_P}/include"
	append-ldflags "-L${BORINGSSL_OUTDIR}"
}

src_configure() {
	local mycmakeargs=(
		"-DCMAKE_BUILD_TYPE=Release"
		"-DCMAKE_SYSTEM_PROCESSOR=${CHOST%%-*}"
		"-DBUILD_SHARED_LIBS=OFF"
	)
	# TODO(b/331820224): We should reenable deprecated-enum-enum-conversion
	# when the offending lines are fixed.
	append-cxxflags "-Wno-error=deprecated-enum-enum-conversion"
	# TODO(b/316021385): We should reenable vla-cxx-extension
	# when the offending lines are fixed.
	append-cxxflags "-Wno-error=vla-cxx-extension"
	cmake_src_configure
	platform_src_configure
}

src_compile() {
	# The build is banned from accessing internet, thus turn off Go Modules
	# to prevent Go from trying to fetch package.
	export GO111MODULE=off
	# Compile BoringSSL and expose libcrypto.a.
	cmake_src_compile

	mkdir -p "${BORINGSSL_OUTDIR}" || die
	cp -p "${BUILD_DIR}/crypto/libcrypto.a" "${BORINGSSL_OUTDIR}/libboringcrypto.a" || die

	platform_src_compile
}

src_install() {
	platform_src_install

	local fuzzer_component_id="157100"
	platform_fuzzer_install "${S}"/OWNERS "${OUT}"/arc_keymintd_fuzzer \
		--comp "${fuzzer_component_id}"
}

platform_pkg_test() {
	platform_test "run" "${OUT}/arc-keymintd_testrunner"
}
