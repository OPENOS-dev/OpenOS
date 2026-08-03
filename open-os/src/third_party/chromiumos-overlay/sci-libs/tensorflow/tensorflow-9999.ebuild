# Copyright 1999-2020 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

PYTHON_COMPAT=( python3_{8..12} )
MY_PV=2.18.0
MY_P=${PN}-${MY_PV}
S="${WORKDIR}/${MY_P}"

# shellcheck disable=SC2034 # Used by bazel.eclass.
BAZEL_BINARY="bazel-6"

CROS_WORKON_PROJECT=(
	"chromiumos/platform/tflite"
)

CROS_WORKON_LOCALNAME=(
	"../platform/tflite"
)

CROS_WORKON_DESTDIR=(
	"${S}"
)

# b/445167094: Work around test failures.
CROS_SANITIZER_DISABLE_UBSAN_FUNCTION=1

inherit cros-bazel cros-sanitizers cros-workon python-r1 flag-o-matic cros-toolchain-funcs udev

DESCRIPTION="Computation framework using data flow graphs for scalable machine learning"
HOMEPAGE="https://www.tensorflow.org/"

LICENSE="Apache-2.0"
SLOT="0/9"
KEYWORDS="~*"
IUSE="
	xnnpack
	tflite_opencl_profiling
	ubsan
	+sample_delegate
	mtk_neuron_delegate
	intel_openvino_delegate
	stable_xnnpack_delegate
	stable_gpu_delegate
	test
"

# distfiles that bazel uses for the workspace, will be copied to bazel-distdir
bazel_external_uris="
	https://github.com/bazelbuild/apple_support/releases/download/1.6.0/apple_support.1.6.0.tar.gz
	https://github.com/bazelbuild/bazel-skylib/releases/download/1.3.0/bazel-skylib-1.3.0.tar.gz
	https://github.com/bazelbuild/rules_android/archive/v0.1.1.zip -> bazelbuild-rules_android-v0.1.1.zip
	https://github.com/bazelbuild/rules_apple/releases/download/2.3.0/rules_apple.2.3.0.tar.gz
	https://github.com/bazelbuild/rules_cc/releases/download/0.0.2/rules_cc-0.0.2.tar.gz
	https://github.com/bazelbuild/rules_closure/archive/308b05b2419edb5c8ee0471b67a40403df940149.tar.gz -> bazelbuild-rules_closure-308b05b2419edb5c8ee0471b67a40403df940149.tar.gz
	https://github.com/bazelbuild/rules_foreign_cc/archive/0.7.1.tar.gz -> bazelbuild-rules_foreign_cc-0.7.1.tar.gz
	https://github.com/bazelbuild/rules_java/releases/download/5.5.1/rules_java-5.5.1.tar.gz
	https://github.com/bazelbuild/rules_jvm_external/archive/4.3.zip -> bazelbuild-rules_jvm_external-4.3.zip
	https://github.com/bazelbuild/rules_pkg/releases/download/0.7.1/rules_pkg-0.7.1.tar.gz -> bazelbuild-rules_pkg-0.7.1.tar.gz
	https://github.com/bazelbuild/rules_proto/archive/11bf7c25e666dd7ddacbcd4d4c4a9de7a25175f8.tar.gz -> bazelbuild-rules_proto-11bf7c25e666dd7ddacbcd4d4c4a9de7a25175f8.tar.gz
	https://github.com/bazelbuild/rules_python/releases/download/0.26.0/rules_python-0.26.0.tar.gz -> bazelbuild-rules_python-0.26.0.tar.gz
	https://github.com/bazelbuild/rules_swift/releases/download/1.5.0/rules_swift.1.5.0.tar.gz -> bazelbuild-rules_swift.1.5.0.tar.gz
	https://github.com/google/benchmark/archive/f7547e29ccaed7b64ef4f7495ecfff1c9f6f3d03.tar.gz -> benchmark-f7547e29ccaed7b64ef4f7495ecfff1c9f6f3d03.tar.gz
	https://github.com/google/farmhash/archive/0d859a811870d10f53a594927d0d0b97573ad06d.tar.gz -> farmhash-0d859a811870d10f53a594927d0d0b97573ad06d.tar.gz
	https://github.com/google/flatbuffers/archive/e6463926479bd6b330cbcf673f7e917803fd5831.tar.gz -> flatbuffers-e6463926479bd6b330cbcf673f7e917803fd5831.tar.gz
	https://github.com/google/gemmlowp/archive/16e8662c34917be0065110bfcd9cc27d30f52fdf.zip -> gemmlowp-16e8662c34917be0065110bfcd9cc27d30f52fdf.zip
	https://github.com/google/googletest/archive/refs/tags/release-1.12.1.tar.gz -> googletest-release-1.12.1.tar.gz
	https://github.com/google/ruy/archive/3286a34cc8de6149ac6844107dfdffac91531e72.zip -> ruy-3286a34cc8de6149ac6844107dfdffac91531e72.zip
	https://github.com/google/XNNPACK/archive/6b83f69d4938da4dc9ad63c00bd13e9695659a51.zip -> XNNPACK-6b83f69d4938da4dc9ad63c00bd13e9695659a51.zip
	https://github.com/googleapis/googleapis/archive/6b3fdcea8bc5398be4e7e9930c693f0ea09316a0.tar.gz -> 6b3fdcea8bc5398be4e7e9930c693f0ea09316a0.tar.gz
	https://github.com/google/highwayhash/archive/c13d28517a4db259d738ea4886b1f00352a3cc33.tar.gz -> highwayhash-c13d28517a4db259d738ea4886b1f00352a3cc33.tar.gz
	https://github.com/indygreg/python-build-standalone/releases/download/20231002/cpython-3.9.18+20231002-x86_64-unknown-linux-gnu-install_only.tar.gz
	https://github.com/intel/ARM_NEON_2_x86_SSE/archive/a15b489e1222b2087007546b4912e21293ea86ff.tar.gz -> ARM_NEON_2_x86_SSE-a15b489e1222b2087007546b4912e21293ea86ff.tar.gz
	https://github.com/jax-ml/ml_dtypes/archive/6f02f77c4fa624d8b467c36d1d959a9b49b07900/ml_dtypes-6f02f77c4fa624d8b467c36d1d959a9b49b07900.tar.gz
	https://github.com/KhronosGroup/OpenCL-Headers/archive/dcd5bede6859d26833cd85f0d6bbcee7382dc9b3.tar.gz -> dcd5bede6859d26833cd85f0d6bbcee7382dc9b3.tar.gz
	https://github.com/KhronosGroup/Vulkan-Headers/archive/32c07c0c5334aea069e518206d75e002ccd85389.tar.gz -> 32c07c0c5334aea069e518206d75e002ccd85389.tar.gz
	https://github.com/llvm/llvm-project/archive/8b4b7d28f7c344c728a9812aa99d9ad24edb40a2.tar.gz -> llvm-project-8b4b7d28f7c344c728a9812aa99d9ad24edb40a2.tar.gz
	https://github.com/Maratyszcza/FP16/archive/4dfe081cf6bcd15db339cf2680b9281b8451eeb3.zip -> FP16-4dfe081cf6bcd15db339cf2680b9281b8451eeb3.zip
	https://github.com/Maratyszcza/FXdiv/archive/63058eff77e11aa15bf531df5dd34395ec3017c8.zip -> FXdiv-63058eff77e11aa15bf531df5dd34395ec3017c8.zip
	https://github.com/Maratyszcza/pthreadpool/archive/4fe0e1e183925bf8cfa6aae24237e724a96479b8.zip -> pthreadpool-4fe0e1e183925bf8cfa6aae24237e724a96479b8.zip
	https://github.com/petewarden/OouraFFT/archive/v1.0.tar.gz -> OouraFFT-v1.0.tar.gz
	https://github.com/pybind/pybind11_bazel/archive/72cbbf1fbc830e487e3012862b7b720001b70672.tar.gz -> pybind11_bazel-72cbbf1fbc830e487e3012862b7b720001b70672.tar.gz
	https://github.com/pytorch/cpuinfo/archive/fa1c679da8d19e1d87f20175ae1ec10995cd3dd3.zip -> pytorch-cpuinfo-fa1c679da8d19e1d87f20175ae1ec10995cd3dd3.zip
	https://github.com/tensorflow/runtime/archive/4e088077c6e455610a39ed8d2a18fe195ada6137.tar.gz -> tensorflow-runtime-4e088077c6e455610a39ed8d2a18fe195ada6137.tar.gz
	https://github.com/tensorflow/tensorflow/archive/v${MY_PV}.tar.gz -> tensorflow-${MY_PV}.tar.gz
	https://gitlab.arm.com/kleidi/kleidiai/-/archive/cddf991af5de49fd34949fa39690e4e906e04074/kleidiai-cddf991af5de49fd34949fa39690e4e906e04074.zip
	https://gitlab.com/libeigen/eigen/-/archive/33d0937c6bdf5ec999939fb17f2a553183d14a74/eigen-33d0937c6bdf5ec999939fb17f2a553183d14a74.tar.gz
	https://storage.googleapis.com/mirror.tensorflow.org/storage.cloud.google.com/download.tensorflow.org/tflite/hexagon_nn_headers_v1.20.0.9.tgz -> hexagon_nn_headers_v1.20.0.9.tgz
"

SRC_URI="${bazel_external_uris}"

RDEPEND="
	dev-cpp/abseil-cpp:=
	>=dev-libs/nsync-1.25.0
	~dev-libs/flatbuffers-24.3.25:=
	dev-libs/openssl:0=
	>=dev-libs/protobuf-3.8.0:=
	>=dev-libs/re2-0.2019.06.01:=
	media-libs/giflib
	media-libs/libjpeg-turbo
	media-libs/libpng:0
	net-misc/curl
	sys-libs/zlib
	virtual/opengles:=
	x11-drivers/opengles-headers:=
	mtk_neuron_delegate? ( virtual/tflite-stable-delegate-deps:= )
	intel_openvino_delegate? ( virtual/tflite-stable-delegate-deps:= )
"
DEPEND="
	${RDEPEND}
"
BDEPEND="
	app-arch/unzip
	~dev-libs/flatbuffers-24.3.25:=
	>=dev-libs/protobuf-3.8.0
	dev-java/java-config
	dev-lang/perl
	=dev-util/bazel-6*
	dev-util/xxd
	${PYTHON_DEPS}
	sys-apps/which
	sys-devel/gettext
"

# Echos the CPU string that TensorFlow uses to refer to the given architecture.
get-cpu-str() {
	local arch
	arch="$(tc-arch "${1}")"

	# Note that `--cpu=arm` is treated as arm64 in tensorflow. Here we use the
	# more specific `armv7a` to align with tensorflow.
	case "${arch}" in
	amd64) echo "k8";;
	arm) echo "armv7a";;
	arm64) echo "aarch64";;
	*) die "Unsupported architecture '${arch}'."
	esac
}

src_unpack() {
	local version
	version="$("${FILESDIR}"/chromeos-version.sh)"
	# The `-` in $MY_PV is replaced with `_` in this comparison, since the
	# version name conventions are different between GitHub and Portage.
	if [[ "${MY_PV//-/_}" != "${version}" ]]; then
		die "mismatched version found between MY_PV and chromeos-version.sh"
	fi

	cros-workon_src_unpack

	bazel_load_distfiles "${bazel_external_uris}"
}

src_prepare() {
	export JAVA_HOME=$(ROOT="${BROOT}" java-config --jdk-home)

	append-lfs-flags
	sanitizers-setup-env
	bazel_setup_bazelrc
	bazel_setup_crosstool "$(get-cpu-str "${CBUILD}")" "$(get-cpu-str "${CHOST}")"

	default
}

src_configure() {
	export JAVA_HOME=$(ROOT="${BROOT}" java-config --jdk-home)

	python_setup
	cros-bazel-add-rc "common --config=cros"
	cros-bazel-add-rc "build --action_env PYTHON_BIN_PATH=${PYTHON}"
	cros-bazel-add-rc "build --action_env PYTHON_LIB_PATH=$(python_get_sitedir)"
	cros-bazel-add-rc "build --python_path=${PYTHON}"

	if use intel_openvino_delegate; then
		cros-bazel-add-rc "build --action_env OPENVINO_DIR=${SYSROOT}/usr/"
	fi

	bazel_setup_system_protobuf

	case "${ARCH}" in
		arm | arm64)
			# The ruy library is faster than the default libeigen on arm, but
			# MUCH slower on amd64. See b/178593695 for more discussion.
			cros-bazel-add-rc 'build --define=tflite_with_ruy=true'
			;;
		amd64 | x86)
			# Detects whether the target CPU supports SSE4.2.
			# Note: the check requires no double quote on CFLAGS and CPPFLAGS.
			# shellcheck disable=SC2086
			if ! tc-cpp-is-true "defined(__SSE4_2__)" ${CFLAGS} ${CPPFLAGS}; then
				cros-bazel-add-copt '-mno-sse4.2'
			fi
			;;
	esac
}

src_compile() {
	export JAVA_HOME=$(ROOT="${BROOT}" java-config --jdk-home)

	local bazel_args=()
	bazel_args+=("@org_tensorflow//tensorflow/lite:libtensorflowlite.so")
	bazel_args+=("@cros_tflite//android:libnativewindow.so")
	bazel_args+=("@org_tensorflow//tensorflow/lite/delegates/utils/experimental/stable_delegate:tflite_settings_json_parser")
	bazel_args+=("@org_tensorflow//tensorflow/lite/delegates/utils/experimental/stable_delegate:delegate_loader")

	if ! use ubsan; then
		bazel_args+=("@org_tensorflow//tensorflow/lite/tools/evaluation/tasks/inference_diff:run_eval")
		bazel_args+=("@org_tensorflow//tensorflow/lite/tools/evaluation/tasks/coco_object_detection:run_eval")
		bazel_args+=("@org_tensorflow//tensorflow/lite/tools/benchmark:benchmark_model")
		bazel_args+=("@org_tensorflow//tensorflow/lite/delegates/utils/experimental/stable_delegate:stable_delegate_test_suite")
		bazel_args+=("@cros_tflite//common:async_delegate_test")
		bazel_args+=("@cros_tflite//tool:decompose_model")
		bazel_args+=("@cros_tflite//android:hardware_buffer_test")
		bazel_args+=("@cros_tflite//tool:op_compat_test")
		bazel_args+=("@cros_tflite//tool:tflite_to_json")
		bazel_args+=("@cros_tflite//tool:npu_top")

		if use sample_delegate; then
			bazel_args+=("@cros_tflite//delegate/sample:tensorflowlite_cros_sample_delegate")
		fi

		if use stable_xnnpack_delegate; then
			bazel_args+=("@org_tensorflow//tensorflow/lite/delegates/utils/experimental/stable_delegate:tensorflowlite_stable_xnnpack_delegate")
		fi

		if use stable_gpu_delegate; then
			bazel_args+=("@cros_tflite//:stable_gpu_delegate")
		fi

		if use mtk_neuron_delegate; then
			bazel_args+=("@cros_tflite//delegate/mtk_neuron:neuron_delegate_test")
		fi
	fi

	if use tflite_opencl_profiling; then
		bazel_args+=("@org_tensorflow//tensorflow/lite/delegates/gpu/cl/testing:performance_profiling")
	fi

	if use mtk_neuron_delegate; then
		bazel_args+=("@cros_tflite//:mtk_neuron_delegate")
	fi

	if use intel_openvino_delegate; then
		bazel_args+=("@cros_tflite//:intel_openvino_delegate")
	fi

	if use test; then
		bazel_args+=("@cros_tflite//android:hardware_buffer_test")
		bazel_args+=("@cros_tflite//common:dump_test")
		bazel_args+=("@cros_tflite//common:log_test")
		bazel_args+=("@cros_tflite//common:minimal_logging_test")
		bazel_args+=("@cros_tflite//common/test_util:fp16_compare_test")
		bazel_args+=("@cros_tflite//delegate/sample:external_test")

		if use intel_openvino_delegate; then
			bazel_args+=("@cros_tflite//delegate/intel_openvino:openvino_delegate_core_test")
			bazel_args+=("@cros_tflite//delegate/intel_openvino:openvino_delegate_test")
			bazel_args+=("@cros_tflite//delegate/intel_openvino:openvino_graph_builder_test")
			bazel_args+=("@cros_tflite//delegate/intel_openvino:openvino_smoke_test")
		fi
	fi

	# fail early if any deps are missing
	ebazel build -k --nobuild "${bazel_args[@]}"
	# build if deps are present
	ebazel build "${bazel_args[@]}"
}

src_install() {
	export JAVA_HOME=$(ROOT="${BROOT}" java-config --jdk-home)

	local external_dir="${S%/}-bazel-base/external"
	local tf_src="${external_dir}/org_tensorflow"
	local tf_bin="bazel-bin/external/org_tensorflow"
	local i

	einfo "Installing TF lite headers"
	local HEADERS_TMP
	HEADERS_TMP="${WORKDIR}/headers_tmp"
	mkdir -p "${HEADERS_TMP}"

	# From tensorflow/lite/lib_package/create_ios_frameworks.sh
	find "${tf_src}/tensorflow/lite" -name "*.h" \
		-not -path "${tf_src}/tensorflow/lite/tools/*" \
		-not -path "${tf_src}/tensorflow/lite/examples/*" \
		-not -path "${tf_src}/tensorflow/lite/gen/*" \
		-not -path "${tf_src}/tensorflow/lite/toco/*" \
		-not -path "${tf_src}/tensorflow/lite/java/*" |
	while read -r i; do
		mkdir -p "${HEADERS_TMP}/${i%/*}"
		cp "${i}" "${HEADERS_TMP}/${i%/*}"
	done

	insinto "/usr/include/${PN}"
	doins -r "${HEADERS_TMP}/${tf_src}/tensorflow"

	einfo "Installing selected TF core headers"
	local selected_core_headers=( lib/bfloat16/bfloat16.h platform/byte_order.h platform/macros.h platform/bfloat16.h )
	for i in "${selected_core_headers[@]}"; do
		insinto "/usr/include/${PN}/${PN}/core/${i%/*}"
		doins "${tf_src}/tensorflow/core/${i}"
	done

	einfo "Installing selected TF MLIR compiler headers"
	local selected_mlir_headers=(
		/allocation.h
		core/api/error_reporter.h
		core/api/verifier.h
		core/macros.h
		core/model_builder_base.h
		experimental/remat/metadata_util.h
		schema/schema_generated.h
		utils/control_edges.h
		utils/string_utils.h
	)
	for i in "${selected_mlir_headers[@]}"; do
		insinto "/usr/include/${PN}/${PN}/compiler/mlir/lite/${i%/*}"
		doins "${tf_src}/tensorflow/compiler/mlir/lite/${i}"
	done

	einfo "Installing ruy headers"
	insinto /usr/include/${PN}/ruy/
	doins -r "${external_dir}/ruy/ruy"/*

	einfo "Installing fp16 headers"
	insinto /usr/include/${PN}/
	doins -r "${external_dir}/FP16/include"/*

	einfo "Installing TF lite libraries"
	dolib.so "${tf_bin}/tensorflow/lite/libtensorflowlite.so"
	# libtflite_settings_json_parser.a is needed for TfLiteSettingsJsonParser
	# and libdelegate_loader.a is needed for LoadDelegateFromSharedLibrary.
	# Both of these are needed to load stable delegates.
	# TODO(b/377439850): If there's any other places than odmld that needs to
	# load stable delegate, then we should move these two library into a single
	# .so that exports these two methods to save space.
	dolib.a "${tf_bin}/tensorflow/lite/delegates/utils/experimental/stable_delegate/libtflite_settings_json_parser.a"
	dolib.a "${tf_bin}/tensorflow/lite/delegates/utils/experimental/stable_delegate/libdelegate_loader.a"

	einfo "Installing libnativewindow.so (AHardwareBuffer API Shim)"
	dolib.so "bazel-bin/android/libnativewindow.so"

	udev_dorules udev/99-tflite.rules

	if use mtk_neuron_delegate; then
		einfo "Installing MTK Neuron Delegate"
		dolib.so "bazel-bin/delegate/mtk_neuron/libtensorflowlite_mtk_neuron_delegate.so"
		udev_dorules udev/99-mtk-npu.rules
		insinto /etc/tflite
		doins delegate/mtk_neuron/settings.json
	fi

	if use intel_openvino_delegate; then
		einfo "Installing Intel OpenVINO Delegate"
		dolib.so "bazel-bin/delegate/intel_openvino/libtensorflowlite_intel_openvino_delegate.so"
		udev_dorules udev/99-intel-npu.rules
		insinto /etc/tflite
		doins delegate/intel_openvino/settings.json
	fi

	if ! use ubsan; then
		into /usr/local
		einfo "Install benchmark_model tool to /usr/local/bin"
		dobin "${tf_bin}/tensorflow/lite/tools/benchmark/benchmark_model"
		einfo "Install inference diff evaluation tool"
		newbin "${tf_bin}/tensorflow/lite/tools/evaluation/tasks/inference_diff/run_eval" inference_diff_eval
		einfo "Install object detection evaluation tool"
		newbin "${tf_bin}/tensorflow/lite/tools/evaluation/tasks/coco_object_detection/run_eval" object_detection_eval
		einfo "Install stable delegate test suite"
		dobin "${tf_bin}/tensorflow/lite/delegates/utils/experimental/stable_delegate/stable_delegate_test_suite"
		einfo "Install async delegate test"
		dobin "bazel-bin/common/async_delegate_test"
		einfo "Install decompose_model tool to /usr/local/bin"
		dobin "bazel-bin/tool/decompose_model"
		einfo "Install tflite_to_json to /usr/local/bin"
		dobin "bazel-bin/tool/tflite_to_json"
		einfo "Install npu_top to /usr/local/bin"
		dobin "bazel-bin/tool/npu_top"
		einfo "Install AHardwareBuffer test"
		newbin "bazel-bin/android/hardware_buffer_test" "android_hardware_buffer_test"
		einfo "Install operators compatibility test"
		newbin "bazel-bin/tool/op_compat_test" "op_compat_test"

		if use mtk_neuron_delegate; then
			einfo "Install NeuronDelegate test"
			newbin "bazel-bin/delegate/mtk_neuron/neuron_delegate_test" "neuron_delegate_test"
		fi

		if use sample_delegate; then
			einfo "Install ChromeOS sample delegate"
			dolib.so "bazel-bin/delegate/sample/libtensorflowlite_cros_sample_delegate.so"
		fi

		if use stable_xnnpack_delegate; then
			einfo "Install stable xnnpack delegate"
			dolib.so "${tf_bin}/tensorflow/lite/delegates/utils/experimental/stable_delegate/libtensorflowlite_stable_xnnpack_delegate.so"
		fi

		if use stable_gpu_delegate; then
			einfo "Install stable gpu delegate"
			dolib.so "bazel-bin/delegate/gpu/libtensorflowlite_stable_gpu_delegate.so"
		fi
	fi

	if use tflite_opencl_profiling; then
		into /usr/local/
		einfo "Install performance_profiling tool"
		dobin "${tf_bin}/tensorflow/lite/delegates/gpu/cl/testing/performance_profiling"
	fi

	if use xnnpack; then
		einfo "Installing XNNPACK headers and libs"
		local bindir="bazel-out/$(get-cpu-str "${CHOST}")-opt/bin/external/"
		insinto /usr/include/${PN}/xnnpack/
		doins "${external_dir}/XNNPACK/include/xnnpack.h"
		doins "${external_dir}/pthreadpool/include/pthreadpool.h"
		dolib.a "${bindir}/cpuinfo/libcpuinfo_impl.pic.a"
		dolib.a "${bindir}/pthreadpool/libpthreadpool.a"
		# The lib names vary wildly between amd64 and arm, so
		# easier just to scan for them rather than explicitly
		# listing them and switching on ${ARCH}.
		find "${bindir}/XNNPACK/" -name "*.a" |
		while read -r i; do
			dolib.a "${i}"
		done
	fi

	einfo "Installing pkg-config file"
	local generate_pc="${tf_src}/tensorflow/lite/generate-pc.sh"
	chmod +x "${generate_pc}"
	"${generate_pc}" --prefix="${EPREFIX}"/usr --libdir="$(get_libdir)" --version=${MY_PV} || die
	insinto /usr/"$(get_libdir)"/pkgconfig
	doins tensorflowlite.pc
}

src_test() {
	export JAVA_HOME=$(ROOT="${BROOT}" java-config --jdk-home)

	local platform2_test_py="${CHROOT_SOURCE_ROOT}/src/platform2/common-mk/platform2_test.py"

	local libdir
	libdir=$(get_libdir)

	# We need to add this path into to LD_LIBRARY_PATH
	# because the cpu plugin for testing is installed in /usr/local.
	local ld_library_path="/usr/local/${libdir}"

	local tests=(
		"//android:hardware_buffer_test"
		"//common:dump_test"
		"//common:log_test"
		"//common:minimal_logging_test"
		"//common/test_util:fp16_compare_test"
		"//delegate/sample:external_test"
	)

	if use intel_openvino_delegate; then
		tests+=("//delegate/intel_openvino:openvino_delegate_core_test")
		tests+=("//delegate/intel_openvino:openvino_delegate_test")
		tests+=("//delegate/intel_openvino:openvino_graph_builder_test")
		tests+=("//delegate/intel_openvino:openvino_smoke_test")
	fi

	local run_flags=(
		# Ensure that all tests are already built in src_compile phase.
		"--check_up_to_date"

		# Run with platform2_test.py so libraries in the sysroot are used.
		"--run_under=${platform2_test_py} --sysroot=${SYSROOT} --env=LD_LIBRARY_PATH=${ld_library_path} --"
	)

	if use amd64; then
		for t in "${tests[@]}"; do
			ebazel run "${run_flags[@]}" "${t}" || die "Test ${t} failed"
		done
	fi
}
