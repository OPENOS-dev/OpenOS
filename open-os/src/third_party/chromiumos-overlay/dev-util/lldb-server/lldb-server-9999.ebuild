# Copyright 2021 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2

EAPI=7

PYTHON_COMPAT=( python3_{8..11} )

CROS_WORKON_REPO="${CROS_GIT_HOST_URL}"
CROS_WORKON_PROJECT="external/github.com/llvm/llvm-project"
CROS_WORKON_LOCALNAME="llvm-project"
CROS_WORKON_OUTOFTREE_BUILD="1"

if [[ "${PV}" == "9999" ]]; then
	# Use incremental builds only for 9999 ebuilds, since caching in the
	# face of toolchain updates can get subtle, and non-9999 builders
	# (mostly bots) won't benefit from keeping cache artifacts around.
	CROS_WORKON_INCREMENTAL_BUILD="1"
fi

inherit cros-constants cmake-multilib git-2 flag-o-matic cros-llvm python-single-r1 cros-toolchain-funcs cros-workon

DESCRIPTION="lldb-server, for the LLDB debugger"
HOMEPAGE="https://github.com/llvm/llvm-project"
SRC_URI=""
EGIT_REPO_URI="${CROS_GIT_HOST_URL}/external/github.com/llvm/llvm-project
	${CROS_GIT_HOST_URL}/external/github.com/llvm/llvm-project"
EGIT_BRANCH=main

LICENSE="|| ( UoI-NCSA MIT )"
SLOT="0"
KEYWORDS="~*"
IUSE="cros_host python local-lldb libedit"
RDEPEND="
	app-arch/xz-utils
	app-arch/zstd
	python? (
		$(python_gen_cond_dep '
			dev-python/six[${PYTHON_USEDEP}]
		')
		${PYTHON_DEPS}
	)
	local-lldb? (
		dev-libs/libxml2
	)
	sys-libs/libcxx
	sys-libs/llvm-libunwind
"

DEPEND="${RDEPEND}
	sys-libs/ncurses
	libedit? (
		dev-libs/libedit
	)
"

BDEPEND="
	${PYTHON_DEPS}
	>=dev-util/cmake-3.16
	python? (
		>=dev-lang/swig-3.0.11
		$(python_gen_cond_dep '
			dev-python/six[${PYTHON_USEDEP}]
		')
	)
"

# CMake build targets that are used neither in the host build or the
# target build.
DISABLE_IRRELEVANT_CMAKE_OPTS=(
	"-DLLDB_ENABLE_CURSES=OFF"
	"-DLLDB_ENABLE_LUA=OFF"
	"-DLLDB_ENABLE_PYTHON=OFF"
	"-DLLDB_INCLUDE_TESTS=OFF"

	"-DLLVM_ENABLE_ZLIB=OFF"
	"-DLLVM_ENABLE_ZSTD=OFF"
	"-DLLVM_INCLUDE_BENCHMARKS=OFF"  # Benchmarks violate portage sandbox
	"-DLLVM_INCLUDE_DOCS=OFF"
	"-DLLVM_INCLUDE_EXAMPLES=OFF"
	"-DLLVM_INCLUDE_RUNTIMES=OFF"
	"-DLLVM_INCLUDE_TESTS=OFF"
	"-DLLVM_INCLUDE_UTILS=OFF"
)

pkg_setup() {
	use cros_host && die "lldb is not supported for building on non-device builds"
	python-single-r1_pkg_setup
	# Setup llvm toolchain for cross-compilation.
	setup_cross_toolchain
	ABI="${ABI:-"${DEFAULT_ABI}"}"
}

src_prepare() {
	python_setup
	cros-llvm_ensure_patches_applied
	export CMAKE_USE_DIR="${S}/llvm"
	eapply_user
	cmake_src_prepare
}

build_llvm_host() {
	einfo "Building LLVM host tablegens."
	local builddir="${WORKDIR}/llvm_build_host"
	local libdir="$(get_libdir)"
	local mycmakeargs=(
		"-DCLANG_BUILD_TOOLS=OFF"
		"-DCLANG_ENABLE_ARCMT=OFF"
		"-DCLANG_ENABLE_STATIC_ANALYZER=OFF"
		"-DCLANG_INCLUDE_DOCS=OFF"
		"-DCLANG_INCLUDE_TESTS=OFF"
		"-DCLANG_LINK_CLANG_DYLIB=OFF"

		"-DCMAKE_BUILD_TYPE=RelWithDebInfo"
		"-DCMAKE_INSTALL_PREFIX=${PREFIX}"

		"-DLLVM_ADD_NATIVE_VISUALIZERS_TO_SOLUTION=OFF"
		"-DLLVM_BUILD_LLVM_DYLIB=OFF"
		"-DLLVM_ENABLE_IDE=ON"
		"-DLLVM_ENABLE_PROJECTS=llvm;clang;lldb"
		"-DLLVM_LIBDIR_SUFFIX=${libdir#lib}"
		"-DLLVM_LINK_LLVM_DYLIB=OFF"
		"-DLLVM_USE_HOST_TOOLS=OFF"

		"-DPython3_EXECUTABLE=${PYTHON}"

		"${DISABLE_IRRELEVANT_CMAKE_OPTS[@]}"
	)
	tc-env_build cmake \
		-B "${builddir}" \
		-GNinja \
		"${mycmakeargs[@]}" \
		"${S}/llvm"
	local targets=(
		llvm-tblgen
		# clang-tblgen is needed, otherwise we encounter
		# issues with using the cross-compiled clang-tblgen.
		# in the cross-compilation.
		clang-tblgen
		lldb-tblgen
	)
	eninja -C "${builddir}" "${targets[@]}"
	einfo "Successfully built LLVM host tablegens."
}

src_configure() {
	# TODO(b/354225301): investigate why this is necessary
	export CMAKE_BUILD_TYPE="RelWithDebInfo"
	build_llvm_host
	local libdir="$(get_libdir)"

	# Get the LLVM name of the target architecture
	case "${CTARGET:?}" in
		aarch64*) local llvm_target_arch="AArch64";;
		arm*) local llvm_target_arch="ARM";;
		x86*) local llvm_target_arch="X86";;
		*) die "Unsupported/unknown CTARGET: ${CTARGET:?}";;
	esac

	local mycmakeargs=(
		"-DCMAKE_CROSSCOMPILING=ON"
		"-DCMAKE_SYSTEM_NAME=Linux"
		"-DCMAKE_SYSTEM_PROCESSOR=${ARCH}"
		"-DCMAKE_INSTALL_PREFIX=${PREFIX}"
		"-DCMAKE_BUILD_TYPE=RelWithDebInfo"
		# Build as a completely standalone executable.
		"-DBUILD_SHARED_LIBS=OFF"

		"-DLLVM_ENABLE_IDE=ON"
		"-DLLVM_ENABLE_PROJECTS=llvm;clang;lldb"

		"-DLLVM_HOST_TRIPLE=${CTARGET:?}"

		"-DLLVM_LIBDIR_SUFFIX=${libdir#lib}"

		# These are why we need the multi-stage build. Without the NATIVE_TOOL_DIR,
		# the LLDB CMake will not know where to look for the *-tblgen files.
		"-DLLDB_TABLEGEN_EXE=${WORKDIR}/llvm_build_host/bin/lldb-tblgen"
		"-DLLVM_DIR=${WORKDIR}/llvm_build_host"
		"-DLLVM_NATIVE_TOOL_DIR=${WORKDIR}/llvm_build_host/bin"
		"-DLLVM_TABLEGEN_EXE=${WORKDIR}/llvm_build_host/bin/llvm-tblgen"
		"-DLLVM_USE_HOST_TOOLS=OFF"
		"-DLLVM_TARGETS_TO_BUILD=${llvm_target_arch}"
		"-DLLDB_ENABLE_LIBEDIT=$(usex libedit)"

		"-DPython3_EXECUTABLE=${PYTHON}"

		"${DISABLE_IRRELEVANT_CMAKE_OPTS[@]}"
	)

	append-flags "-Oz"

	cmake_src_configure
}

src_compile() {
	local targets=(
		lldb-server
	)
	if use "local-lldb"; then
		targets+=(lldb)
	fi
	cmake_src_compile "${targets[@]}"
}

src_install() {
	# shellcheck disable=SC2154
	dobin "${BUILD_DIR}"/bin/lldb-server

	if use "local-lldb"; then
		local ins_libdir="$(get_libdir)"
		dobin "${BUILD_DIR}"/bin/lldb
		insinto "/usr/${ins_libdir}"
		doins "${BUILD_DIR}/${ins_libdir}/liblldb.so.19.0.0git"
		doins "${BUILD_DIR}/${ins_libdir}/liblldb.so.19.0git"
		doins "${BUILD_DIR}/${ins_libdir}/liblldb.so"
	fi
}
