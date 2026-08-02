# Copyright 1999-2023 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

: "${CMAKE_MAKEFILE_GENERATOR:=ninja}"
PYTHON_COMPAT=( python3_{8..11} )

inherit cmake flag-o-matic python-single-r1

DESCRIPTION="OpenCL C Library"
HOMEPAGE="https://libclc.llvm.org/"
LICENSE="Apache-2.0-with-LLVM-exceptions || ( MIT BSD )"

SRC_URI="https://github.com/llvm/llvm-project/releases/download/llvmorg-${PV}/llvm-project-${PV}.src.tar.xz"
SLOT="0/${PV}"
KEYWORDS="-* amd64"
IUSE=""

BDEPEND="
	${PYTHON_DEPS}
	~dev-util/spirv-llvm-${PV}
"

S="${WORKDIR}/llvm-project-${PV}.src"
CMAKE_USE_DIR="${S}/libclc"

src_prepare() {
	python_setup
	cmake_src_prepare
}

src_configure() {
	local spirv_llvm="/opt/spirv-llvm"
	append-cppflags "-I${spirv_llvm}/include"
	append-ldflags "-L${spirv_llvm}/lib"

	local mycmakeargs=(
		"-DLLVM_DIR=${spirv_llvm}/lib/cmake/llvm"
		"-DLIBCLC_TARGETS_TO_BUILD=spirv-mesa3d-;spirv64-mesa3d-"
		"-DLLVM_AS=${spirv_llvm}/bin/llvm-as"
		"-DLLVM_CLANG=${spirv_llvm}/bin/clang"
		"-DLLVM_CONFIG=${spirv_llvm}/bin/llvm-config"
		"-DLLVM_LINK=${spirv_llvm}/bin/llvm-link"
		"-DLLVM_OPT=${spirv_llvm}/bin/opt"
		"-DLLVM_SPIRV=${spirv_llvm}/bin/llvm-spirv"
	)
	if [[ -z "${CCACHE_DISABLE:-}" ]]; then
		# If ccache is enabled, use it as a compiler launcher.
		mycmakeargs+=(
			"-DCMAKE_C_COMPILER_LAUNCHER=ccache"
			"-DCMAKE_CXX_COMPILER_LAUNCHER=ccache"
		)
	fi
	cmake_src_configure
}
