# Copyright 2015 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

inherit cmake cros-sanitizers

DESCRIPTION="drawElements Quality Program - an OpenGL ES testsuite"
HOMEPAGE="https://github.com/KhronosGroup/VK-GL-CTS"

# This corresponds to a commit for the chosen tag/branch.
MY_DEQP_COMMIT='24c1b1498ba4f05777f47541968ffe686265c645'

# When building the Vulkan CTS, dEQP requires that certain external
# dependencies be unpacked into the source tree. See ${S}/external/fetch_sources.py
# in the dEQP for the required dependencies. Upload these tarballs to the ChromeOS mirror too and
# update the manifest.
MY_AMBER_COMMIT='0f003c2785489f59cd01bb2440fcf303149100f2'
MY_GLSLANG_COMMIT='2b19bf7e1bc0b60cf2fe9d33e5ba6b37dfc1cc83'
MY_JSONCPP_COMMIT='9059f5cad030ba11d37818847443a53918c327b1'
MY_SPIRV_TOOLS_COMMIT='4c7e1fa5c3d988cca0e626d359d30b117b9c2822'
MY_SPIRV_HEADERS_COMMIT='db5a00f8cebe81146cafabf89019674a3c4bf03d'
MY_NVIDIA_VIDEO_SAMPLES_COMMIT='6821adf11eb4f84a2168264b954c170d03237699'

SRC_URI="
	https://github.com/KhronosGroup/VK-GL-CTS/archive/${MY_DEQP_COMMIT}.tar.gz -> deqp-${MY_DEQP_COMMIT}.tar.gz
	https://github.com/KhronosGroup/glslang/archive/${MY_GLSLANG_COMMIT}.tar.gz -> glslang-${MY_GLSLANG_COMMIT}.tar.gz
	https://github.com/KhronosGroup/SPIRV-Tools/archive/${MY_SPIRV_TOOLS_COMMIT}.tar.gz -> SPIRV-Tools-${MY_SPIRV_TOOLS_COMMIT}.tar.gz
	https://github.com/KhronosGroup/SPIRV-Headers/archive/${MY_SPIRV_HEADERS_COMMIT}.tar.gz -> SPIRV-Headers-${MY_SPIRV_HEADERS_COMMIT}.tar.gz
	https://github.com/google/amber/archive/${MY_AMBER_COMMIT}.tar.gz -> amber-${MY_AMBER_COMMIT}.tar.gz
	https://github.com/open-source-parsers/jsoncpp/archive/${MY_JSONCPP_COMMIT}.tar.gz -> jsoncpp-${MY_JSONCPP_COMMIT}.tar.gz
	https://github.com/nvpro-samples/vk_video_samples/archive/${MY_NVIDIA_VIDEO_SAMPLES_COMMIT}.tar.gz -> nvidia-video-samples-${MY_NVIDIA_VIDEO_SAMPLES_COMMIT}.tar.gz
"

LICENSE="Apache-2.0"
SLOT="0"
KEYWORDS="*"
IUSE="ubsan vulkan"

BDEPEND="app-arch/zstd"
RDEPEND="
	virtual/opengles
	media-libs/minigbm
	media-libs/libpng
	vulkan? ( virtual/vulkan-icd )

	!<media-gfx/deqp-runner-0.18.0
"

DEPEND="${RDEPEND}
	x11-drivers/opengles-headers
	x11-libs/libX11
"

S="${WORKDIR}"

PATCHES=(
	"${FILESDIR}"/FROMLIST-Remove-extended_usage_bit_compatibility-with-image_f.patch
	"${FILESDIR}"/0001-FROMLIST-Fix-instances-of-Wmissing-template-arg-list-after-te.patch
	"${FILESDIR}"/emit-AndroidHardwareBufferExternalApi-dtor.patch
	"${FILESDIR}"/vk14.patch
	"${FILESDIR}"/fix-missing-includes.patch
)

src_unpack() {
	default_src_unpack || die

	mv "VK-GL-CTS-${MY_DEQP_COMMIT}/"* .
	mkdir -p external/{amber,glslang,spirv-tools,spirv-headers}
	mv "amber-${MY_AMBER_COMMIT}" external/amber/src || die
	mv "jsoncpp-${MY_JSONCPP_COMMIT}" external/jsoncpp/src || die
	mv "glslang-${MY_GLSLANG_COMMIT}" external/glslang/src || die
	mv "SPIRV-Tools-${MY_SPIRV_TOOLS_COMMIT}" external/spirv-tools/src || die
	mv "SPIRV-Headers-${MY_SPIRV_HEADERS_COMMIT}" external/spirv-headers/src || die
	mv "vk_video_samples-${MY_NVIDIA_VIDEO_SAMPLES_COMMIT}" external/nvidia-video-samples/src || die
}

src_prepare() {
	cros_enable_cxx_exceptions

	cmake_src_prepare

	pushd android/cts/main > /dev/null || die
	for api in egl gles2 gles3 gles31; do
		sort -u ${api}-main-*.txt | zstd -c > tmp_cat_${api}-main.txt.zst || die
	done
	if use vulkan; then
		cat vk-main-*.txt | xargs sort -u | zstd -c > tmp_cat_vk-main.txt.zst || die
	fi
	popd > /dev/null || die
}

src_configure() {
	sanitizers-setup-env

	# See crbug.com/585712.
	append-lfs-flags

	local de_cpu=
	case "${ARCH}" in
		x86)   de_cpu='DE_CPU_X86';;
		amd64) de_cpu='DE_CPU_X86_64';;
		arm)   de_cpu='DE_CPU_ARM';;
		arm64) de_cpu='DE_CPU_ARM_64';;
		*) die "unknown ARCH '${ARCH}'";;
	esac

	# Override dEQP's list of default build targets. Do not build Vulkan SC.
	local build_targets=(
		deqp-egl
		deqp-gles2
		deqp-gles3
		deqp-gles31
		testlog-to-xml
	)
	if use vulkan; then
		build_targets+=( deqp-vk )
	fi

	# Tell cmake to not produce rpaths. crbug.com/585715.
	local mycmakeargs=(
		-DCMAKE_SKIP_RPATH=1
		-DCMAKE_FIND_ROOT_PATH="${ROOT}"
		-DCMAKE_FIND_ROOT_PATH_MODE_PROGRAM=NEVER
		-DCMAKE_FIND_ROOT_PATH_MODE_LIBRARY=ONLY
		-DCMAKE_FIND_ROOT_PATH_MODE_INCLUDE=ONLY
		-DSELECTED_BUILD_TARGETS="${build_targets[*]}"
		-DDE_CPU="${de_cpu}"
		-DDEQP_TARGET=surfaceless
		-DGLES_ALLOW_DIRECT_LINK=OFF
		-DBUILD_SHARED_LIBS=OFF
		-Wno-dev
	)

	if use ubsan; then
		mycmakeargs+=(
			-DENABLE_RTTI=1
			-DAMBER_ENABLE_RTTI=1
		)
	fi

	append-cxxflags "-DQP_SUPPORT_PNG=1"

	cmake_src_configure
}

src_install() {
	# dEQP requires that the layout of its installed files match the layout
	# of its build directory. Otherwise the binaries cannot find the data
	# files.
	local deqp_dir="/usr/local/${PN}"

	# Install module binaries
	exeinto "${deqp_dir}/modules/egl"
	doexe "${BUILD_DIR}/modules/egl/deqp-egl"
	exeinto "${deqp_dir}/modules/gles2"
	doexe "${BUILD_DIR}/modules/gles2/deqp-gles2"
	exeinto "${deqp_dir}/modules/gles3"
	doexe "${BUILD_DIR}/modules/gles3/deqp-gles3"
	exeinto "${deqp_dir}/modules/gles31"
	doexe "${BUILD_DIR}/modules/gles31/deqp-gles31"
	if use vulkan; then
		exeinto "${deqp_dir}/external/vulkancts/modules/vulkan"
		doexe "${BUILD_DIR}/external/vulkancts/modules/vulkan/deqp-vk"
	fi

	# Install tools
	exeinto "${deqp_dir}/executor"
	doexe "${BUILD_DIR}/executor/testlog-to-xml"

	# Install data files
	insinto "${deqp_dir}/modules/gles2"
	doins -r "${BUILD_DIR}/modules/gles2/gles2"
	insinto "${deqp_dir}/modules/gles3"
	doins -r "${BUILD_DIR}/modules/gles3/gles3"
	insinto "${deqp_dir}/modules/gles31"
	doins -r "${BUILD_DIR}/modules/gles31/gles31"
	if use vulkan; then
		insinto "${deqp_dir}/external/vulkancts/modules/vulkan"
		doins -r "${BUILD_DIR}/external/vulkancts/modules/vulkan/vulkan"
	fi
	insinto "${deqp_dir}"
	doins -r "doc/testlog-stylesheet"

	# Install caselists
	insinto "${deqp_dir}/caselists"
	for api in egl gles2 gles3 gles31; do
		newins "android/cts/main/tmp_cat_${api}-main.txt.zst" "${api}.txt.zst"
	done
	if use vulkan; then
		newins "android/cts/main/tmp_cat_vk-main.txt.zst" "vk.txt.zst"
	fi
}
