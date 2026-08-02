# Copyright 2023 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

inherit cmake

DESCRIPTION="A tool which profiles OpenCL devices to find their peak capacities"
HOMEPAGE="https://github.com/krrishnarraj/clpeak/"

LICENSE="public-domain"
SLOT="0"
KEYWORDS="*"
IUSE=""

CLPEAK_ARCHIVE_NAME="clpeak-1.1.5"
CLPEAK_ARCHIVE="${CLPEAK_ARCHIVE_NAME}.zip"
SRC_URI="https://storage.googleapis.com/chromeos-localmirror/distfiles/${CLPEAK_ARCHIVE}"

# target build dependencies
DEPEND="
	>=dev-libs/clhpp-2023.02.06
	>=dev-util/opencl-headers-2023.02.06
	>=media-libs/clvk-0.0.1
"

# target runtime dependencies
RDEPEND="
	>=media-libs/clvk-0.0.1
"

# host build dependencies
BDEPEND="
	app-arch/unzip
	>=dev-util/cmake-3.13.4
"

CMAKE_USE_DIR="${WORKDIR}/${CLPEAK_ARCHIVE_NAME}"

src_unpack() {
	unpack "${CLPEAK_ARCHIVE}"
	mkdir -p "${WORKDIR}/${P}"
	pushd "${CMAKE_USE_DIR}" || die
	# Apply patches
	#eapply "${FILESDIR}/<some_patch>" || die
	popd || die
}

src_prepare() {
	cros_enable_cxx_exceptions
	cmake_src_prepare
}

src_configure() {
	append-lfs-flags

	cmake_src_configure
}

src_install() {
	local OPENCL_TESTS_DIR="/usr/local/opencl"
	dodir "${OPENCL_TESTS_DIR}"
	exeinto "${OPENCL_TESTS_DIR}"

	doexe "${BUILD_DIR}/clpeak"
}
