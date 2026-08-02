# Copyright 2023 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

EAPI=7

inherit cmake cros-sanitizers

DESCRIPTION="Perfetto-based Vulkan Shader Profiler"
HOMEPAGE="https://github.com/rjodinchr/vulkan-shader-profiler"

VKSP_ARCHIVE_NAME="vulkan-shader-profiler-11702948e90f301b9302982714ab4ab6412a4de0"
VKSP_ARCHIVE="${VKSP_ARCHIVE_NAME}.zip"

SPIRV_VERSION="1.4.313.0"
SRC_URI="
gs://chromeos-localmirror/distfiles/${VKSP_ARCHIVE}
https://github.com/KhronosGroup/spirv-tools/archive/vulkan-sdk-${SPIRV_VERSION}.tar.gz -> spirv-tools-${SPIRV_VERSION}.tar.gz
"

CMAKE_USE_DIR="${WORKDIR}/${VKSP_ARCHIVE_NAME}"

LICENSE="Apache-2.0"
SLOT="0"
KEYWORDS="*"
IUSE="+perfetto"

RDEPEND="
	media-libs/vulkan-loader
	>=dev-util/spirv-tools-${SPIRV_VERSION}
"
DEPEND="
	>=dev-util/spirv-headers-${SPIRV_VERSION}
	dev-util/vulkan-headers
	>=chromeos-base/perfetto-31.0
	${RDEPEND}
"

# Need to unpack the source archive.
BDEPEND="app-arch/unzip"

src_unpack() {
	unpack "spirv-tools-${SPIRV_VERSION}.tar.gz"
	unpack "${VKSP_ARCHIVE}"
	mkdir -p "${WORKDIR}/${P}"
}

src_configure() {
	sanitizers-setup-env
	append-lfs-flags
	# shellcheck disable=SC2034
	local mycmakeargs=(
		-DBACKEND=System
		-DPERFETTO_SDK_PATH="${ESYSROOT}/usr/include/perfetto/"
		-DPERFETTO_LIBRARY=perfetto_sdk
		-DPERFETTO_INTERNAL_INCLUDE_PATH="${ESYSROOT}/usr/include/perfetto/"
		-DPERFETTO_TRACE_PROCESSOR_LIB="${ESYSROOT}/usr/local/$(get_libdir)/libtrace_processor.a"
		-DSPIRV_TOOLS_SOURCE_PATH="${WORKDIR}/SPIRV-Tools-vulkan-sdk-${SPIRV_VERSION}"
		-DSPIRV_TOOLS_BUILD_PATH="${ESYSROOT}/usr/include/spirv-tools/vksp"
	)
	cmake_src_configure
}

src_install() {
	local LAYER_DIR="/usr/local/share/vulkan/explicit_layer.d/"
	dodir "${LAYER_DIR}"
	insinto "${LAYER_DIR}"
	doins "${CMAKE_USE_DIR}/manifest/${PN}.json"

	dolib.so "${BUILD_DIR}/layer/lib${PN}.so"

	dobin "${BUILD_DIR}/extractor/${PN}-extractor"
	dobin "${BUILD_DIR}/runner/${PN}-runner"
	dobin "${BUILD_DIR}/merge-buffers/${PN}-merge-buffers"

	exeinto "/usr/local/bin"
	doexe "${CMAKE_USE_DIR}/chromeos-utils/${PN}.sh"
	dosym "/usr/local/bin/${PN}.sh" "/usr/local/bin/vksp.sh"
}
