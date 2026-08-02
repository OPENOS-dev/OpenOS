# Copyright 1999-2017 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Id$

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

DESCRIPTION="Compiler runtime library for clang"
HOMEPAGE="http://compiler-rt.llvm.org/"

LICENSE="LLVM-exception"
SLOT="0"
KEYWORDS="*"
IUSE="+llvm-crt"

BDEPEND="
	sys-devel/llvm
	${PYTHON_DEPS}
"

if [[ ${CATEGORY} == cross-* ]] ; then
	# TODO(b/317118942): Remove the need for gcc.
	BDEPEND+="
		${CATEGORY}/binutils
		sys-libs/libcxx
	"
fi
if [[ ${CATEGORY} == cross-*linux-gnu* ]] ; then
	DEPEND+="
		${CATEGORY}/libxcrypt
		${CATEGORY}/linux-headers
	"
fi

if is_baremetal_abi && [[ ${CTARGET} == riscv* ]]; then
	export CROSTC_USER_ACKNOWLEDGES_THAT_RISCV_IS_EXPERIMENTAL=1
fi

src_prepare() {
	python_setup

	cros-llvm_ensure_patches_applied

	# Since compiler-rt is moving to runtimes,
	# we should build with CMAKE there.
	export CMAKE_USE_DIR="${S}/runtimes"
	cmake_src_prepare
}

src_configure() {
	setup_cross_toolchain
	append-flags "-fomit-frame-pointer"
	# CTARGET is defined in an eclass, which shellcheck won't see
	# shellcheck disable=SC2154
	if [[ ${CTARGET} == armv7a* ]]; then
		# Use vfpv3 to be able to target non-neon targets
		append-flags -mfpu=vfpv3
	fi
	BUILD_DIR=${WORKDIR}/${P}_build

	# b/309410154: Ensure we're disabling assertions w/ debug info.
	append-flags "-DNDEBUG" "-g"

	local mycmakeargs=(
		"-DLLVM_ENABLE_RUNTIMES=compiler-rt"
		"-DCMAKE_TRY_COMPILE_TARGET_TYPE=STATIC_LIBRARY"
		# crbug/855759
		"-DCOMPILER_RT_BUILD_CRT=$(usex llvm-crt)"
		"-DCOMPILER_RT_USE_LIBCXX=yes"
		"-DCOMPILER_RT_LIBCXXABI_PATH=${S}/libcxxabi"
		"-DCOMPILER_RT_LIBCXX_PATH=${S}/libcxx"
		"-DCOMPILER_RT_HAS_GNU_VERSION_SCRIPT_COMPAT=no"
		"-DCOMPILER_RT_BUILTINS_HIDE_SYMBOLS=OFF"
		# b/200831212: Disable per runtime install dirs.
		"-DLLVM_ENABLE_PER_TARGET_RUNTIME_DIR=OFF"
		# b/204220308: Disable ORC since we are not using it.
		"-DCOMPILER_RT_BUILD_ORC=OFF"
		"-DCOMPILER_RT_INSTALL_PATH=${EPREFIX}$(${CC} --print-resource-dir)"
	)

	if is_baremetal_abi; then
		# Options for baremetal toolchains e.g. armv7m-cros-eabi.
		append-flags -Oz # Optimize for smallest size.

		mycmakeargs+=(
			"-DCMAKE_C_COMPILER_TARGET=${CTARGET}"
			"-DCMAKE_POSITION_INDEPENDENT_CODE=OFF"
			"-DCOMPILER_RT_BAREMETAL_BUILD=yes"
			"-DCOMPILER_RT_BUILD_CRT=OFF"
			# Disable sanitizers, profilers, and fuzzers for baremetal
			# As they do not work without an OS.
			"-DCOMPILER_RT_BUILD_LIBFUZZER=no"
			"-DCOMPILER_RT_BUILD_SANITIZERS=no"
			"-DCOMPILER_RT_BUILD_XRAY=no"
			"-DCOMPILER_RT_BUILD_MEMPROF=no"
			"-DCOMPILER_RT_BUILD_CTX_PROFILE=no"
			"-DCOMPILER_RT_BUILD_PROFILE=no"

			"-DCOMPILER_RT_BUILTINS_ENABLE_PIC=OFF"
			"-DCOMPILER_RT_DEFAULT_TARGET_ONLY=yes"
			"-DCOMPILER_RT_OS_DIR=baremetal"
		)
		# TODO(b/416568977): It would be more flexible to set up multilib so
		# that applications can customize the flags they want (CPU/arch,
		# soft/hard float, etc.)
		if [[ ${CTARGET} == arm-none-eabi ]]; then
			# b/205342596: This is a hack to provide armv6m builtins for use with
			# arm-none-eabi without creating a separate armv6m toolchain.
			append-flags "-march=armv6m --sysroot=/usr/arm-none-eabi"
			mycmakeargs+=( "-DCMAKE_C_COMPILER_TARGET=armv6m-none-eabi" )
		elif [[ "${CTARGET}" == armv7m-cros-eabi ]]; then
			# b/286910996: Set target-specific floating point flags.
			append-flags -mcpu=cortex-m4
			append-flags -mfloat-abi=hard
		elif [[ ${CTARGET} == riscv* ]]; then
			# b/416568282: Set target-specific floating point flags.
			append-flags -march=rv32imfc
		fi
	else
		# Standard userspace toolchains e.g. armv7a-cros-linux-gnueabihf.
		mycmakeargs+=(
			"-DCOMPILER_RT_BUILD_LIBFUZZER=yes"
			"-DCOMPILER_RT_BUILD_SANITIZERS=yes"
			"-DCOMPILER_RT_DEFAULT_TARGET_TRIPLE=${CTARGET}"
			"-DCOMPILER_RT_SANITIZERS_TO_BUILD=asan;msan;hwasan;tsan;cfi;ubsan_minimal;gwp_asan"
			"-DCOMPILER_RT_TEST_TARGET_TRIPLE=${CTARGET}"
		)
	fi
	cmake_src_configure
}

src_install() {
	# There is install conflict between cross-armv7a-cros-linux-gnueabihf
	# and cross-armv7a-cros-linux-gnueabi. Remove this once we are ready to
	# move to cross-armv7a-cros-linux-gnueabihf.
	if [[ ${CTARGET} == armv7a-cros-linux-gnueabi ]] ; then
		return
	fi
	cmake_src_install

	# includes and docs are installed for all sanitizers and xray
	# These files conflict with files provided in llvm ebuild
	local libdir=$(llvm-config --libdir)
	rm -rf "${ED}"/usr/share || die
	rm -rf "${ED}${libdir}"/clang/*/include || die
	rm -f "${ED}${libdir}"/clang/*/*list.txt || die
	rm -f "${ED}${libdir}"/clang/*/*/*list.txt || die
	rm -f "${ED}${libdir}"/clang/*/dfsan_abilist.txt || die
	rm -f "${ED}${libdir}"/clang/*/*/dfsan_abilist.txt || die
	rm -f "${ED}${libdir}"/clang/*/bin/* || die

	if is_baremetal_abi; then
		# Verify that no relocations are generated for baremetal.
		local elf_file had_failures=false
		while read -r elf_file; do
			if $(tc-getREADELF) --relocs "${elf_file}" | grep GOT; then
				eerror "Unexpected GOT relocations found in ${elf_file}"
				had_failures=true
			fi
		done < <(scanelf -RByF '%F' "${D}")
		"${had_failures}" && die "GOT relocations found in baremetal"
	fi

	# Copy compiler-rt files to a new clang version to handle llvm updates gracefully.
	local llvm_version=$(llvm-config --version)
	local clang_full_version=${llvm_version%svn*}
	clang_full_version=${clang_full_version%git*}
	local major_version=${clang_full_version%%.*}
	[[ -d "${D}${libdir}/clang/${major_version}" ]] || die "Could not find installed compiler-rt files."

	# N.B., many files here are installed in a directory with LLVM's major version
	# in the path name. For a while, we had logic to duplicate this directory for
	# convenience during LLVM upgrades/bisections, but it was removed. For discussion
	# on how to readd it if it's wanted, see comments on https://crrev.com/c/6075304.

	# b/281531340 we install libclang_rt.builtins-${arch}.a. For armv6m,
	# this is libclang_rt.builtins-armv6m.a, but we have historically
	# used arm-none-eabi-clang as the compiler driver, which looks for
	# libclang_rt.builtins-arm.a instead ("arm" vs. "armv6m"). To
	# make this work, add a symlink.
	if [[ "${CATEGORY}" == "cross-arm-none-eabi" ]]; then
		ln -s libclang_rt.builtins-armv6m.a \
			"${D}${libdir}/clang/${major_version}/lib/baremetal/libclang_rt.builtins-arm.a" || die
	fi
}
