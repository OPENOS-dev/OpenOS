# Copyright 2021 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

inherit autotools flag-o-matic cmake

DESCRIPTION="Utility for generating AFDO profiles"
HOMEPAGE="http://gcc.gnu.org/wiki/AutoFDO"

GIT_COMMIT=3dafe34db0eb53af146cf782124f788ceaf6a9aa
GLOG_COMMIT=0.6.0
PERF_DATA_CONVERTER_COMMIT=b665ecebcb0f14988408036422ac114cade65a7c
SRC_URI="
https://github.com/google/${PN}/archive/${GIT_COMMIT}.tar.gz -> ${PN}-${GIT_COMMIT}.tar.gz
https://github.com/google/glog/archive/${GLOG_COMMIT}.tar.gz -> glog-${GLOG_COMMIT}.tar.gz
https://github.com/google/perf_data_converter/archive/${PERF_DATA_CONVERTER_COMMIT}.tar.gz \
	-> perf_data_converter-${PERF_DATA_CONVERTER_COMMIT}.tar.gz
"

LICENSE="Apache-2.0"
SLOT="0"
KEYWORDS="*"
IUSE=""

DEPEND="
	app-arch/zstd:=
	dev-cpp/abseil-cpp:=
	dev-cpp/gflags
	dev-libs/elfutils:=
	dev-libs/openssl:0=
	dev-libs/protobuf:=
	dev-libs/libffi
	sys-devel/llvm:=
	sys-libs/ncurses:0=
	sys-libs/zlib"

RDEPEND="${DEPEND}"

PATCHES=(
	"${FILESDIR}/autofdo_cmake.patch"
	"${FILESDIR}/autofdo_merge_result.patch"
	"${FILESDIR}/autofdo_lazy_evaluator.patch"
	"${FILESDIR}/autofdo-0.30-update-sec-layout-function.patch"
)

src_unpack() {
	default
	ln -s "${PN}-${GIT_COMMIT}" "${P}" || die

	# Replace the third_party/glog directory with our extracted glog sources.
	rmdir "${S}/third_party/glog" || die
	ln -s "${WORKDIR}/glog-${GLOG_COMMIT}" "${S}/third_party/glog" || die

	# Replace the perf_data_converter directory with our extracted sources.
	rmdir "${S}/third_party/perf_data_converter" || die
	ln -s "${WORKDIR}/perf_data_converter-${PERF_DATA_CONVERTER_COMMIT}" \
		"${S}/third_party/perf_data_converter" || die

	# Remove the bundled absl and use the system absl headers.
	# Symlink the the system headers so they are available at the
	# expected local path.
	rm -r "${S}/third_party/abseil" || die
	mkdir -p "${S}/third_party/abseil" || die
	ln -s "${SYSROOT}/usr/include/absl" "${S}/third_party/abseil/"
}

src_prepare() {
	eapply_user
	cmake_src_prepare
}

src_configure() {
	tc-export PKG_CONFIG
	export AUTOFDO_LIBDIR="/usr/$(get_libdir)/autofdo"
	append-ldflags "$(no-as-needed)" "$(${PKG_CONFIG} protobuf --libs)"
	append-ldflags "-Wl,--rpath=${AUTOFDO_LIBDIR}"
	append-cxxflags "-std=gnu++20"
	# shellcheck disable=SC2034
	local mycmakeargs=(
		"-DLLVM_TARGETS_TO_BUILD=X86;ARM32;AARCH64"
		"-DBUILD_SHARED=ON"
		"-DBUILD_TESTING=OFF"
		"-DINSTALL_GTEST=OFF"
		"-DLLVM_PATH=$(llvm-config --cmakedir)"
		"-DWITH_LLVM=ON"
	)
	cmake_src_configure
}

src_compile() {
	cmake_src_compile create_llvm_prof profile_merger sample_merger
}

src_install() {
	dobin "${BUILD_DIR}"/create_llvm_prof "${BUILD_DIR}"/profile_merger \
		"${BUILD_DIR}"/sample_merger

	# Install the shared libs needed to link in a hacky autofdo lib subdir.
	exeinto "${AUTOFDO_LIBDIR}"
	insinto "${AUTOFDO_LIBDIR}"

	# While dev-cpp/glog exists, autofdo's cmake does not work without the actual source code
	# of glog. This is Bad, but we don't have a way to fix this immediately. Any fix needs
	# to go upstream so that it won't break us the next time we upgrade autofdo.
	doexe "${BUILD_DIR}"/third_party/glog/libglog.so."${GLOG_COMMIT}"
	# Symlink, not executable.
	doins "${BUILD_DIR}"/third_party/glog/libglog.so.1

	# Could consider moving these outside of the /usr/lib*/autofdo directory, but ultimately
	# they aren't causing a problem within the lib subdir.
	doexe "${BUILD_DIR}"/libllvm_propeller_options_proto.so
	doexe "${BUILD_DIR}"/libllvm_propeller_cfg_proto.so
	doexe "${BUILD_DIR}"/libperf_proto.so
}
