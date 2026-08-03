# Copyright 2023 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

EAPI=7

CROS_WORKON_COMMIT="686356bf458d2441c36d62ce8f047486329ec666"
CROS_WORKON_TREE="85edb12d8c585841ab8052c7d69ecaff50aeafe8"
PYTHON_COMPAT=( python3_{8..11} )

CROS_WORKON_REPO="${CROS_GIT_HOST_URL}"
CROS_WORKON_PROJECT="external/github.com/llvm/llvm-project"
CROS_WORKON_LOCALNAME="llvm-project"
CROS_WORKON_OUTOFTREE_BUILD="1"

# Build somewhat differently when a dev is iterating on LLVM.
if [[ "${PV}" == "9999" ]]; then
	# Use incremental builds only for 9999 ebuilds, since caching in the
	# face of toolchain updates can get subtle, and non-9999 builders
	# (mostly bots) won't benefit from keeping cache artifacts around.
	CROS_WORKON_INCREMENTAL_BUILD="1"
fi

inherit eutils cros-toolchain-funcs cros-constants cmake git-2 cros-llvm cros-workon python-single-r1

EGIT_REPO_URI="${CROS_GIT_HOST_URL}/external/github.com/llvm/llvm-project
	${CROS_GIT_HOST_URL}/external/github.com/llvm/llvm-project"
EGIT_BRANCH=main

DESCRIPTION="LLVM scudo_standalone memory allocator"
HOMEPAGE="http://compiler-rt.llvm.org/"

LICENSE="LLVM-exception"
SLOT="0"
KEYWORDS="*"
IUSE="system_wide_scudo"
BDEPEND="
	sys-devel/llvm
	${PYTHON_DEPS}
"
DEPEND="sys-libs/libxcrypt"

RDEPEND="
	sys-libs/libcxx
	sys-libs/llvm-libunwind
"

src_prepare() {
	python_setup

	cros-llvm_ensure_patches_applied

	export CMAKE_USE_DIR="${S}/compiler-rt"
	eapply_user
	cmake_src_prepare
}

src_configure() {
	BUILD_DIR="${WORKDIR}/${P}_build"
	append-lfs-flags
	append-flags -DUSE_CHROMEOS_CONFIG

	# mycmakeargs is used by cmake_src_configure, but shellcheck doesn't see
	# this.
	# shellcheck disable=SC2034
	local mycmakeargs=(
		"-DCOMPILER_RT_BUILD_CRT=no"
		"-DCOMPILER_RT_USE_LIBCXX=yes"
		"-DCOMPILER_RT_LIBCXXABI_PATH=${S}/libcxxabi"
		"-DCOMPILER_RT_LIBCXX_PATH=${S}/libcxx"
		"-DCOMPILER_RT_HAS_GNU_VERSION_SCRIPT_COMPAT=no"
		"-DCOMPILER_RT_BUILTINS_HIDE_SYMBOLS=OFF"
		"-DCOMPILER_RT_BUILD_SANITIZERS=yes"
		"-DCOMPILER_RT_BUILD_LIBFUZZER=no"
		"-DCOMPILER_RT_DEFAULT_TARGET_TRIPLE=${CTARGET}"
		"-DCOMPILER_RT_TEST_TARGET_TRIPLE=${CTARGET}"

		# We require gwp_asan as we want it built within the scudo dso
		"-DCOMPILER_RT_SANITIZERS_TO_BUILD=scudo_standalone;gwp_asan"
		"-DLLVM_ENABLE_PER_TARGET_RUNTIME_DIR=OFF"
		"-DCOMPILER_RT_BUILD_ORC=OFF"
		"-DCOMPILER_RT_INSTALL_PATH=${EPREFIX}$(${CC} --print-resource-dir)"
	)

	cmake_src_configure
}

src_install() {
	local arch
	case "${ARCH}" in
		x86) arch='i386';;
		amd64) arch='x86_64';;
		arm) arch='armhf';;
		arm64) arch='aarch64';;
		*) die "unknown ARCH '${ARCH}'";;
	esac

	# Install the scudo_standalone .so
	local libname="libclang_rt.scudo_standalone-${arch}.so"
	dolib.so "${BUILD_DIR}/lib/linux/${libname}"
}
