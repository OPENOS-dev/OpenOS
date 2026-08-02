# Copyright 2024 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

inherit cmake unpacker

DESCRIPTION="Intel OpenVino Toolkit with NPUX support"
HOMEPAGE="https://github.com/openvinotoolkit/openvino"

# Note: We are using the exact commit hash instead of the actual version of OpenVINO 24.5.0 because NPU v1.10.0 was released with OpenVINO_2024.5.0_rev.0ebff04.
SRC_URI="
	https://github.com/openvinotoolkit/openvino/archive/0ebff040fd22daa37612a82fdf930ffce4ebb099.tar.gz -> openvino-0ebff040fd22daa37612a82fdf930ffce4ebb099.tar.gz
	https://github.com/herumi/xbyak/archive/refs/tags/v7.05.tar.gz -> xbyak-7.05.tar.gz
	https://github.com/zeux/pugixml/archive/2e357d19a3228c0a301727aac6bea6fecd982d21.tar.gz -> pugixml-2e357d19a3228c0a301727aac6bea6fecd982d21.tar.gz
	https://github.com/google/snappy/archive/refs/tags/1.2.1.tar.gz -> snappy-1.2.1.tar.gz
	https://github.com/openvinotoolkit/oneDNN/archive/1ce2d722922efb80da52a6efe2152a9aecdddebf.tar.gz -> oneDNN-1ce2d722922efb80da52a6efe2152a9aecdddebf.tar.gz
	https://github.com/madler/zlib/archive/refs/tags/v1.3.1.tar.gz -> zlib-1.3.1.tar.gz
	https://github.com/gflags/gflags/archive/refs/tags/v2.2.2.tar.gz -> gflags-2.2.2.tar.gz
	https://github.com/nlohmann/json/archive/refs/tags/v3.11.3.tar.gz -> json-3.11.3.tar.gz
	https://github.com/ARM-software/ComputeLibrary/archive/refs/tags/v24.08.tar.gz -> ComputeLibrary-24.08.tar.gz
	https://github.com/openvinotoolkit/mlas/archive/d1bc25ec4660cddd87804fcf03b2411b5dfb2e94.tar.gz -> mlas-d1bc25ec4660cddd87804fcf03b2411b5dfb2e94.tar.gz
	https://github.com/oneapi-src/level-zero/archive/refs/tags/v1.17.6.tar.gz -> level-zero-1.17.6.tar.gz
	https://github.com/intel/level-zero-npu-extensions/archive/cdb761dd63b1d47230d501e631a2d725db09ba0d.tar.gz -> level-zero-npu-extensions-cdb761dd63b1d47230d501e631a2d725db09ba0d.tar.gz
	https://github.com/libxsmm/libxsmm/archive/13df674c4b73a1b84f6456de8595903ebfbb43e0.tar.gz -> libxsmm-13df674c4b73a1b84f6456de8595903ebfbb43e0.tar.gz
	https://github.com/openvinotoolkit/shl/archive/27992eaf41ef967ed228ea8d801b1aa489ea8997.tar.gz -> shl-27992eaf41ef967ed228ea8d801b1aa489ea8997.tar.gz
	https://github.com/openvinotoolkit/npu_plugin/archive/refs/tags/npu_ud_2024_44_rc1.tar.gz -> npu_plugin-npu_ud_2024_44_rc1.tar.gz
	https://github.com/openvinotoolkit/npu_plugin_elf/archive/98f6fc4e93c0aca2c7620a32bd5c684b515f8532.tar.gz -> npu_plugin_elf-98f6fc4e93c0aca2c7620a32bd5c684b515f8532.tar.gz
	https://github.com/google/flatbuffers/archive/refs/tags/v2.0.6.tar.gz -> flatbuffers-2.0.6.tar.gz
	https://github.com/google/gtest-parallel/archive/f4d65b555894b301699c7c3c52906f72ea052e83.tar.gz -> gtest-parallel-f4d65b555894b301699c7c3c52906f72ea052e83.tar.gz
	https://github.com/intel/npu-plugin-llvm/archive/2811ba3472ec30a5dbb967015c7d9737dd595937.tar.gz -> npu-plugin-llvm-2811ba3472ec30a5dbb967015c7d9737dd595937.tar.gz
	https://github.com/intel/npu-nn-cost-model/archive/cc99ccf8b77ca59e5c1b1a923a7b58f7721ff39c.tar.gz -> npu-nn-cost-model-cc99ccf8b77ca59e5c1b1a923a7b58f7721ff39c-with-lfs.tar.gz
	https://github.com/jbeder/yaml-cpp/archive/0e6e28d1a38224fc8172fae0109ea7f673c096db.tar.gz -> yaml-cpp-0e6e28d1a38224fc8172fae0109ea7f673c096db.tar.gz
"
LICENSE="Apache-2.0"
KEYWORDS="-* amd64"
SLOT="0"

S="${WORKDIR}/openvino-0ebff040fd22daa37612a82fdf930ffce4ebb099"

RDEPEND="
	app-arch/zstd
	dev-cpp/gflags
	dev-cpp/gtest
	dev-libs/flatbuffers
"

DEPEND="
	${RDEPEND}
"

CMAKE_BUILD_TYPE="Release"

src_unpack() {
	unpack openvino-0ebff040fd22daa37612a82fdf930ffce4ebb099.tar.gz

	unpack xbyak-7.05.tar.gz
	rm -r "${S}/thirdparty/xbyak/"
	mv  xbyak-7.05 "${S}/thirdparty/xbyak"

	unpack pugixml-2e357d19a3228c0a301727aac6bea6fecd982d21.tar.gz
	rm -r "${S}/thirdparty/pugixml/"
	mv  pugixml-2e357d19a3228c0a301727aac6bea6fecd982d21 "${S}/thirdparty/pugixml"

	unpack snappy-1.2.1.tar.gz
	rm -r "${S}/thirdparty/snappy/"
	mv  snappy-1.2.1 "${S}/thirdparty/snappy"

	unpack oneDNN-1ce2d722922efb80da52a6efe2152a9aecdddebf.tar.gz
	rm -r "${S}/src/plugins/intel_cpu/thirdparty/onednn/"
	mv  oneDNN-1ce2d722922efb80da52a6efe2152a9aecdddebf "${S}/src/plugins/intel_cpu/thirdparty/onednn"

	unpack zlib-1.3.1.tar.gz
	rm -r "${S}/thirdparty/zlib/zlib/"
	mv  zlib-1.3.1 "${S}/thirdparty/zlib/zlib"

	unpack gflags-2.2.2.tar.gz
	rm -r "${S}/thirdparty/gflags/gflags/"
	mv  gflags-2.2.2 "${S}/thirdparty/gflags/gflags"

	unpack json-3.11.3.tar.gz
	rm -r "${S}/thirdparty/json/nlohmann_json/"
	mv  json-3.11.3 "${S}/thirdparty/json/nlohmann_json"

	unpack ComputeLibrary-24.08.tar.gz
	rm -r "${S}/src/plugins/intel_cpu/thirdparty/ComputeLibrary/"
	mv  ComputeLibrary-24.08 "${S}/src/plugins/intel_cpu/thirdparty/ComputeLibrary"

	unpack mlas-d1bc25ec4660cddd87804fcf03b2411b5dfb2e94.tar.gz
	rm -r "${S}/src/plugins/intel_cpu/thirdparty/mlas/"
	mv  mlas-d1bc25ec4660cddd87804fcf03b2411b5dfb2e94 "${S}/src/plugins/intel_cpu/thirdparty/mlas"

	unpack level-zero-1.17.6.tar.gz
	rm -r "${S}/thirdparty/level_zero/level-zero/"
	mv  level-zero-1.17.6 "${S}/thirdparty/level_zero/level-zero"

	unpack level-zero-npu-extensions-cdb761dd63b1d47230d501e631a2d725db09ba0d.tar.gz
	rm -r "${S}/src/plugins/intel_npu/thirdparty/level-zero-ext"
	mv  level-zero-npu-extensions-cdb761dd63b1d47230d501e631a2d725db09ba0d	 "${S}/src/plugins/intel_npu/thirdparty/level-zero-ext"

	unpack libxsmm-13df674c4b73a1b84f6456de8595903ebfbb43e0.tar.gz
	rm -r "${S}/src/plugins/intel_cpu/thirdparty/libxsmm"
	mv  libxsmm-13df674c4b73a1b84f6456de8595903ebfbb43e0 "${S}/src/plugins/intel_cpu/thirdparty/libxsmm"

	unpack shl-27992eaf41ef967ed228ea8d801b1aa489ea8997.tar.gz
	rm -r "${S}/src/plugins/intel_cpu/thirdparty/shl"
	mv  shl-27992eaf41ef967ed228ea8d801b1aa489ea8997 "${S}/src/plugins/intel_cpu/thirdparty/shl"

	unpack npu_plugin-npu_ud_2024_44_rc1.tar.gz

	unpack npu_plugin_elf-98f6fc4e93c0aca2c7620a32bd5c684b515f8532.tar.gz
	rm -r "${S}/../npu_plugin-npu_ud_2024_44_rc1/thirdparty/elf"
	mv npu_plugin_elf-98f6fc4e93c0aca2c7620a32bd5c684b515f8532 "${S}/../npu_plugin-npu_ud_2024_44_rc1/thirdparty/elf"

	unpack gtest-parallel-f4d65b555894b301699c7c3c52906f72ea052e83.tar.gz
	rm -r "${S}/../npu_plugin-npu_ud_2024_44_rc1/thirdparty/gtest-parallel"
	mv gtest-parallel-f4d65b555894b301699c7c3c52906f72ea052e83 "${S}/../npu_plugin-npu_ud_2024_44_rc1/thirdparty/gtest-parallel"

	unpack npu-plugin-llvm-2811ba3472ec30a5dbb967015c7d9737dd595937.tar.gz
	rm -r "${S}/../npu_plugin-npu_ud_2024_44_rc1/thirdparty/llvm-project"
	mv npu-plugin-llvm-2811ba3472ec30a5dbb967015c7d9737dd595937 "${S}/../npu_plugin-npu_ud_2024_44_rc1/thirdparty/llvm-project"

	unpack npu-nn-cost-model-cc99ccf8b77ca59e5c1b1a923a7b58f7721ff39c-with-lfs.tar.gz
	rm -r "${S}/../npu_plugin-npu_ud_2024_44_rc1/thirdparty/vpucostmodel"
	mv npu-nn-cost-model-cc99ccf8b77ca59e5c1b1a923a7b58f7721ff39c "${S}/../npu_plugin-npu_ud_2024_44_rc1/thirdparty/vpucostmodel"

	unpack yaml-cpp-0e6e28d1a38224fc8172fae0109ea7f673c096db.tar.gz
	rm -r "${S}/../npu_plugin-npu_ud_2024_44_rc1/thirdparty/yaml-cpp"
	mv yaml-cpp-0e6e28d1a38224fc8172fae0109ea7f673c096db "${S}/../npu_plugin-npu_ud_2024_44_rc1/thirdparty/yaml-cpp"
}

src_prepare() {
	# CrOS: Remove the upstream flatc dependency because it gets cross-compiled for targets
	# (it runs on the host and breaks on some machines), significantly increases build times
	# and is already available and better maintained in portage (eg 2.0.6 vs 24.3.25)
	cd "${S}/../npu_plugin-npu_ud_2024_44_rc1" || die
	eapply "${FILESDIR}/0003-PATCH-flatbuffers-use-host-flatc.patch"
	eapply "${FILESDIR}/0004-Fix-libcxx-compatibility-RDFRegisters.patch"

	# Apply patches for Openvino
	cd "${S}" || die
	eapply "${FILESDIR}/0001-PATCH-Openvino-changes-for-2024.5.0_rev.0ebff04-on-C.patch"

	# Apply patches for level-zero
	cd "${S}/thirdparty/level_zero/level-zero" || die
	eapply "${FILESDIR}/0003-Fix-Wdeprecated-literal-operator.patch"

	cros_enable_cxx_exceptions
	eapply_user
	cmake_src_prepare
}

build_llvm_host() {
	einfo "Building LLVM host tablegens."
	local builddir="${WORKDIR}/llvm_build_host"
	local libdir="$(get_libdir)"
	local mycmakeargs=(
		"-DCMAKE_BUILD_TYPE=Release"
		"-DLLVM_BUILD_LLVM_DYLIB=OFF"
		"-DLLVM_ENABLE_IDE=ON"
		"-DLLVM_ENABLE_PROJECTS=llvm"
		"-DLLVM_LIBDIR_SUFFIX=${libdir#lib}"
		"-DLLVM_LINK_LLVM_DYLIB=OFF"
		"-DLLVM_USE_HOST_TOOLS=OFF"
		"-DLLVM_ENABLE_ZLIB=OFF"
		"-DLLVM_ENABLE_ZSTD=OFF"
		"-DLLVM_INCLUDE_BENCHMARKS=OFF"
		"-DLLVM_INCLUDE_DOCS=OFF"
		"-DLLVM_INCLUDE_EXAMPLES=OFF"
		"-DLLVM_INCLUDE_RUNTIMES=OFF"
		"-DLLVM_INCLUDE_TESTS=OFF"
		"-DLLVM_INCLUDE_UTILS=OFF"
	)
	tc-env_build cmake \
		-B"${builddir}" \
		-GNinja \
		"${mycmakeargs[@]}" \
		"${S}/../npu_plugin-npu_ud_2024_44_rc1/thirdparty/llvm-project/llvm"
	eninja -C "${builddir}" llvm-tblgen
	einfo "Successfully built LLVM host tablegens."
}

src_configure() {
	build_llvm_host

	cros_enable_cxx_exceptions
	append-flags "-Wno-undef -frtti -fvisibility=default -Wno-macro-redefined -D__CHROMIUMOS__ -Wno-unqualified-std-cast-call"

	# mycmakeargs is used by cmake_src_configure.
	# shellcheck disable=SC2034
	local mycmakeargs=(
		-GNinja
		-DCMAKE_BUILD_TYPE=Release
		-DCMAKE_INSTALL_PREFIX=/usr/local
		-DDNNL_ENABLE_WORKLOAD="INFERENCE"
		-DENABLE_AUTO=OFF
		-DENABLE_AUTO_BATCH=OFF
		-DENABLE_INTEL_CPU=ON
		-DENABLE_INTEL_GNA=OFF
		-DENABLE_INTEL_GPU=OFF
		-DENABLE_INTEL_MYRIAD_COMMON=OFF
		-DENABLE_JS=OFF
		-DENABLE_HETERO=OFF
		-DENABLE_MULTI=OFF
		-DENABLE_NCC_STYLE=OFF
		-DENABLE_OPENCV=OFF
		-DENABLE_OV_ONNX_FRONTEND=OFF
		-DENABLE_OV_PADDLE_FRONTEND=OFF
		-DENABLE_OV_PYTORCH_FRONTEND=OFF
		-DENABLE_OV_TF_FRONTEND=OFF
		-DENABLE_OV_TF_LITE_FRONTEND=OFF
		-DENABLE_PYTHON=OFF
		-DENABLE_SYSTEM_FLATBUFFERS=ON
		-DTARGET_OS_NAME="CHROMIUMOS"
		-DTHREADING=SEQ
	)
	cmake_src_configure
}

src_compile() {
	cmake_src_compile

	local builddir="${S}/../npu_plugin-npu_ud_2024_44_rc1/build"
	append-flags "-frtti -Wno-error,-Wno-dtor-name -I${S}/samples/cpp/common/format_reader -w"
	cmake -GNinja -DOpenVINODeveloperPackage_DIR="${BUILD_DIR}" \
		-B"${builddir}" \
		-Dflatc_COMMAND="/usr/bin/flatc" \
		-Dflatc_TARGET="" \
		-DBUILD_COMPILER_FOR_DRIVER=ON \
		-DBUILD_SHARED_LIBS=OFF \
		-DCMAKE_BUILD_TYPE=Release \
		-DENABLE_DEVELOPER_BUILD=ON \
		-DLLVM_ENABLE_ZLIB=OFF \
		-DLLVM_NATIVE_TOOL_DIR="${WORKDIR}/llvm_build_host/bin" \
		-DTARGET_OS_NAME="CHROMIUMOS" \
		"${S}"/../npu_plugin-npu_ud_2024_44_rc1
	eninja -C "${builddir}"
}

src_install() {
	# Install headers into /usr/include
	insinto /usr/include/openvino
	doins -r "${S}"/src/core/include/openvino
	doins -r "${S}"/src/frontends/common/include/openvino
	doins -r "${S}"/src/bindings/c/include/openvino
	doins -r "${S}"/src/inference/include/openvino

	# Install libraries
	into /usr/
	dolib.so "${S}"/bin/intel64/"${CMAKE_BUILD_TYPE}"/libopenvino.so.2024.5.0
	dosym libopenvino.so.2024.5.0 /usr/lib64/libopenvino.so.2450
	dosym libopenvino.so.2450 /usr/lib64/libopenvino.so

	dolib.so "${S}"/bin/intel64/"${CMAKE_BUILD_TYPE}"/libopenvino_ir_frontend.so.2024.5.0
	dosym libopenvino_ir_frontend.so.2024.5.0 /usr/lib64/libopenvino_ir_frontend.so.2450

	dolib.so "${S}"/bin/intel64/"${CMAKE_BUILD_TYPE}"/libopenvino_c.so.2024.5.0
	dosym libopenvino_c.so.2024.5.0 /usr/lib64/libopenvino_c.so
	dosym libopenvino_c.so /usr/lib64/libopenvino_c.so.2450

	dolib.so "${S}"/bin/intel64/"${CMAKE_BUILD_TYPE}"/libnpu_driver_compiler.so
	dolib.so "${S}"/bin/intel64/"${CMAKE_BUILD_TYPE}"/libopenvino_intel_npu_plugin.so

	# Install plugins config
	insinto /etc/openvino
	doins "${FILESDIR}"/plugins.xml

	# CPU plugin is used in unit tests on host.
	into /usr/local/
	dolib.so "${S}"/bin/intel64/"${CMAKE_BUILD_TYPE}"/libopenvino_intel_cpu_plugin.so

	# Install test binaries
	exeinto /usr/local/bin
	newexe "${S}"/bin/intel64/"${CMAKE_BUILD_TYPE}"/hello_query_device ov_hello_query_device
	newexe "${S}"/bin/intel64/"${CMAKE_BUILD_TYPE}"/benchmark_app ov_benchmark_app
	newexe "${S}"/bin/intel64/"${CMAKE_BUILD_TYPE}"/compile_tool ov_compile_tool
}
