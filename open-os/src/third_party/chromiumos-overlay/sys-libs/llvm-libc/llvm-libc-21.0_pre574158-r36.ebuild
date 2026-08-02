# Copyright 2023 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

EAPI=7

CROS_WORKON_COMMIT="686356bf458d2441c36d62ce8f047486329ec666"
CROS_WORKON_TREE="85edb12d8c585841ab8052c7d69ecaff50aeafe8"
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

inherit eutils cros-toolchain-funcs cros-constants cmake git-2 cros-llvm cros-workon

EGIT_REPO_URI="${CROS_GIT_HOST_URL}/external/github.com/llvm/llvm-project
	${CROS_GIT_HOST_URL}/external/github.com/llvm/llvm-project"
EGIT_BRANCH=main

DESCRIPTION="LLVM libc"
HOMEPAGE="https://libc.llvm.org"

LICENSE="LLVM-exception"
SLOT="0"
KEYWORDS="*"
IUSE=""
DEPEND=""
BDEPEND="sys-devel/llvm"

pkg_setup() {
	if [[ "${CATEGORY}" == "sys-libs" ]]; then
		die "Host build detected. Don't install llvm-libc for hosts."
	fi
	setup_cross_toolchain
}

src_prepare() {
	cros-llvm_ensure_patches_applied

	export CMAKE_USE_DIR="${S}/llvm"
	eapply_user
	cmake_src_prepare
}

src_configure() {
	append-lfs-flags
	append-cppflags "-DNDEBUG"

	local libdir="$(get_libdir)"
	# shellcheck disable=SC2034 # mycmakeargs is used by cmake.eclass.
	local mycmakeargs=(
		"-GNinja"
		"-DLLVM_ENABLE_PROJECTS=libc"
		"-DCMAKE_BUILD_TYPE=MinSizeRel"
		"-DLLVM_LIBC_INCLUDE_SCUDO=OFF"
		"-DLLVM_LIBC_FULL_BUILD=ON"
		"-DLIBC_INCLUDE_BENCHMARKS=OFF"
		"-DLLVM_LIBDIR_SUFFIX=${libdir#lib}"
	)

	cmake_src_configure
}

src_compile() {
	cmake_build libc
}

src_install() {
	# shellcheck disable=SC2154 # CTARGET is defined in cros-llvm.eclass.
	DESTDIR="${D}/usr/${CTARGET}" cmake_build install-libc || die
}
