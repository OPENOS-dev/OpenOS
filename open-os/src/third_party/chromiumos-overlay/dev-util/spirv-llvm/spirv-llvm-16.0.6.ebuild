# Copyright 1999-2023 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

: "${CMAKE_MAKEFILE_GENERATOR:=ninja}"
PYTHON_COMPAT=( python3_{8..11} )

inherit cmake flag-o-matic python-single-r1 pax-utils cros-toolchain-funcs

# NOTE: this is independent from the 'main' sys-devel/llvm ebuild, since these
# tools have not yet been updated to work with opaque pointers. See b/307937217.
DESCRIPTION="LLVM distribution for SPIRV-related tools"
HOMEPAGE="https://llvm.org/"
LICENSE="LLVM-exception"

SPIRV_LLVM_TRANSLATOR_VERSION="$(ver_cut 1).0.0"
SPIRV_HEADERS_VERSION="1.3.261.0"
SRC_URI="
	https://github.com/llvm/llvm-project/releases/download/llvmorg-${PV}/llvm-project-${PV}.src.tar.xz
	https://github.com/KhronosGroup/SPIRV-LLVM-Translator/archive/refs/tags/v${SPIRV_LLVM_TRANSLATOR_VERSION}.tar.gz -> spirv-llvm-translator-${SPIRV_LLVM_TRANSLATOR_VERSION}.tar.gz
	https://github.com/KhronosGroup/SPIRV-Headers/archive/sdk-${SPIRV_HEADERS_VERSION}.tar.gz -> spirv-headers-${SPIRV_HEADERS_VERSION}.tar.gz
"

SLOT="0/${PV}"
KEYWORDS="-* amd64"
IUSE=""

COMMON_DEPEND="
	app-arch/zstd
	dev-util/spirv-tools
	sys-libs/zlib
"
# configparser-3.2 breaks the build (3.3 or none at all are fine)
DEPEND="
	${COMMON_DEPEND}
	sys-devel/binutils
"
RDEPEND="
	${COMMON_DEPEND}
	${PYTHON_DEPS}
"
BDEPEND="
	${PYTHON_DEPS}
	app-arch/xz-utils
	sys-devel/gnuconfig
	sys-libs/binutils-libs
	sys-libs/libxcrypt
	$(python_gen_cond_dep 'dev-python/dataclasses[${PYTHON_USEDEP}]' python3_6)
"

S="${WORKDIR}/llvm-project-${PV}.src"
CMAKE_USE_DIR="${S}/llvm"
INSTALL_PREFIX="/opt/spirv-llvm"

PATCHES=(
	"${FILESDIR}/spirv-llvm-add-spirv-files-to-distribution-component.patch"
	"${FILESDIR}/spirv-llvm-dont-look-up-backtrace.patch"
	"${FILESDIR}/spirv-llvm-add-type-traits-to-demangle.patch"
)

src_unpack() {
	unpack "llvm-project-${PV}.src.tar.xz"
	(
		cd "${S}/llvm/projects" || die
		unpack "spirv-llvm-translator-${SPIRV_LLVM_TRANSLATOR_VERSION}.tar.gz"
		mv SPIRV-LLVM-Translator{-"${SPIRV_LLVM_TRANSLATOR_VERSION}",} || die
		unpack "spirv-headers-${SPIRV_HEADERS_VERSION}.tar.gz"
		mv SPIRV-Headers{-sdk-"${SPIRV_HEADERS_VERSION}",} || die
	)
}

src_prepare() {
	python_setup
	cmake_src_prepare
}

src_configure() {
	export CMAKE_BUILD_TYPE="RelWithDebInfo"
	append-flags -Wno-poison-system-directories

	local dist_components=(
		LLVMSPIRVLib
		clang
		clang-cmake-exports
		clang-headers
		clang-libraries
		cmake-exports
		llvm-as
		llvm-config
		llvm-headers
		llvm-libraries
		llvm-link
		llvm-spirv
		opencl-resource-headers
		opt
	)

	local mycmakeargs=(
		"${mycmakeargs[@]}"
		"-DLLVM_ENABLE_PROJECTS=llvm;clang"
		"-DLLVM_DISTRIBUTION_COMPONENTS=$(IFS=';'; echo "${dist_components[*]}")"
		"-DLLVM_LINK_LLVM_DYLIB=OFF"
		"-DLLVM_ENABLE_TERMINFO=OFF"
		"-DCMAKE_INSTALL_PREFIX=${INSTALL_PREFIX}"
		"-DBUILD_SHARED_LIBS=OFF"
		"-DLLVM_TARGETS_TO_BUILD=X86"
		"-DLLVM_INCLUDE_BENCHMARKS=OFF"
		"-DLLVM_INCLUDE_EXAMPLES=OFF"
		"-DLLVM_BUILD_TESTS=OFF"
		"-DLLVM_ENABLE_FFI=NO"
		"-DLLVM_ENABLE_ASSERTIONS=NO"
		"-DLLVM_ENABLE_EH=ON"
		"-DLLVM_ENABLE_RTTI=ON"
		"-DLLVM_HOST_TRIPLE=${CHOST}"
		"-DLLVM_BINUTILS_INCDIR=${SYSROOT}/usr/include"
		"-DLLVM_ENABLE_LIBCXX=YES"
		"-DLLVM_USE_LINKER=lld"
		"-DCMAKE_POSITION_INDEPENDENT_CODE=ON"
		"-DLLVM_ENABLE_PER_TARGET_RUNTIME_DIR=OFF"
		"-DPython3_EXECUTABLE=${PYTHON}"
		"-DOCAMLFIND=NO"
	)

	if [[ -z "${CCACHE_DISABLE:-}" ]]; then
		mycmakeargs+=(
			"-DCMAKE_C_COMPILER_LAUNCHER=ccache"
			"-DCMAKE_CXX_COMPILER_LAUNCHER=ccache"
		)
	fi

	cmake_src_configure
}

src_compile() {
	cmake_src_compile distribution
}

src_install() {
	into "${INSTALL_PREFIX}"
	DESTDIR="${D}" cmake_build install-distribution
}
