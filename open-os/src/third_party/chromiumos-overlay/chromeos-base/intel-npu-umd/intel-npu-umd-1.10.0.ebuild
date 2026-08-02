# Copyright 2024 The ChromiumOS Authors
# Distributed under the terms of the MIT License

EAPI=7

inherit cmake flag-o-matic unpacker cros-toolchain-funcs

DESCRIPTION="Intel(R) Neural Processing Unit User-mode driver"
HOMEPAGE="https://github.com/intel/linux-npu-driver"
# This is temporary solution as of now to push additional intel-npu-umd-1.10.0-r1-npu-driver-fw-lfs-files.tar.gz to local mirror.
# Next Opensource release npu-fw will be convert from lfs file to bin file So we would get correct size for vpu_37xx_v0.0.bin in source archive itself.
SRC_URI="
	https://github.com/intel/linux-npu-driver/archive/v${PV}.tar.gz -> ${P}.tar.gz
	https://github.com/openvinotoolkit/npu_plugin_elf/archive/98f6fc4e93c0aca2c7620a32bd5c684b515f8532.tar.gz -> npu_plugin_elf-98f6fc4e93c0aca2c7620a32bd5c684b515f8532.tar.gz
	https://github.com/oneapi-src/level-zero/archive/refs/tags/v1.17.44.tar.gz -> level-zero-1.17.44.tar.gz
	https://github.com/google/googletest/archive/refs/tags/v1.14.0.tar.gz -> googletest-1.14.0.tar.gz
	https://github.com/jbeder/yaml-cpp/archive/refs/tags/0.8.0.tar.gz -> yaml-cpp-0.8.0.tar.gz
	https://github.com/intel/level-zero-npu-extensions/archive/a63155ae4e64feaaa6931f4696c2e2e699063875.tar.gz -> level-zero-npu-extensions-a63155ae4e64feaaa6931f4696c2e2e699063875.tar.gz
	https://commondatastorage.googleapis.com/chromeos-localmirror/intel-npu-umd-1.10.0-r1-npu-driver-fw-lfs-files.tar.gz -> intel-npu-umd-1.10.0-r1-npu-driver-fw-lfs-files.tar.gz
"

LICENSE="MIT"
SLOT="0"
KEYWORDS="-* amd64"

S="${WORKDIR}/linux-npu-driver-${PV}"

DEPEND=""

RDEPEND="
	${DEPEND}
	!chromeos-base/intel-vpu-umd
	dev-libs/openssl
	sys-fs/udev
"

CMAKE_BUILD_TYPE="Release"

src_unpack() {
	unpack ${P}.tar.gz

	unpack intel-npu-umd-1.10.0-r1-npu-driver-fw-lfs-files.tar.gz
	rm  "${S}/firmware/bin/vpu_37xx_v0.0.bin"
	mv vpu_37xx_v0.0.bin "${S}/firmware/bin/vpu_37xx_v0.0.bin"

	unpack npu_plugin_elf-98f6fc4e93c0aca2c7620a32bd5c684b515f8532.tar.gz
	rm -r "${S}/third_party/vpux_elf"
	mv npu_plugin_elf-98f6fc4e93c0aca2c7620a32bd5c684b515f8532 "${S}/third_party/vpux_elf"

	unpack level-zero-1.17.44.tar.gz
	rm -r "${S}/third_party/level-zero"
	mv level-zero-1.17.44 "${S}/third_party/level-zero"

	unpack googletest-1.14.0.tar.gz
	rm -r "${S}/third_party/googletest"
	mv googletest-1.14.0 "${S}/third_party/googletest"

	unpack yaml-cpp-0.8.0.tar.gz
	rm -r "${S}/third_party/yaml-cpp"
	mv yaml-cpp-0.8.0 "${S}/third_party/yaml-cpp"

	unpack level-zero-npu-extensions-a63155ae4e64feaaa6931f4696c2e2e699063875.tar.gz
	rm -r "${S}/third_party/level-zero-npu-extensions"
	mv level-zero-npu-extensions-a63155ae4e64feaaa6931f4696c2e2e699063875 "${S}/third_party/level-zero-npu-extensions"
}

src_prepare() {
	eapply "${FILESDIR}/0001-PATCH-Remove-OpenCV-dependency-on-npu-umd.patch"
	cros_enable_cxx_exceptions
	eapply_user
	cmake_src_prepare
}

src_configure() {
	cros_enable_cxx_exceptions

	# mycmakeargs is used by cmake_src_configure.
	# shellcheck disable=SC2034
	local mycmakeargs=(
		-DSKIP_UNIT_TESTS=ON
		-DBUILD_SHARED_LIBS=OFF
		-DENABLE_OPENCV=OFF
	)
	cmake_src_configure
}

src_install() {
	# Install libraries
	dolib.so "${BUILD_DIR}/lib/libze_intel_vpu.so.1.10.0"
	dosym libze_intel_vpu.so.1.10.0 "/usr/$(get_libdir)/libze_intel_vpu.so.1"
	dosym libze_intel_vpu.so.1 "/usr/$(get_libdir)/libze_intel_vpu.so"

	dolib.so "${BUILD_DIR}/lib/libze_loader.so.1.17.44"
	dosym libze_loader.so.1.17.44 "/usr/$(get_libdir)/libze_loader.so.1"
	dosym libze_loader.so.1 "/usr/$(get_libdir)/libze_loader.so"

	dolib.so "${BUILD_DIR}/lib/libze_validation_layer.so.1.17.44"
	dosym libze_validation_layer.so.1.17.44 "/usr/$(get_libdir)/libze_validation_layer.so.1"
	dosym libze_validation_layer.so.1 "/usr/$(get_libdir)/libze_validation_layer.so"

	# Install test binary
	exeinto /usr/local/bin
	doexe "${BUILD_DIR}/bin/npu-umd-test"

	# Install firmware
	insinto /lib/firmware/intel/vpu
	doins "${S}/firmware/bin/vpu_37xx_v0.0.bin"
}
