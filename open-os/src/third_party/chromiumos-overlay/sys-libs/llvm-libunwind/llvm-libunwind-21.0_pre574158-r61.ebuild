# Copyright 1999-2017 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2

# Cache bust: 2

EAPI=7

CROS_WORKON_COMMIT="686356bf458d2441c36d62ce8f047486329ec666"
CROS_WORKON_TREE="85edb12d8c585841ab8052c7d69ecaff50aeafe8"
PYTHON_COMPAT=( python3_11 )

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

inherit cros-fuzzer cros-sanitizers cros-constants cmake-multilib flag-o-matic git-2 cros-llvm cros-workon python-any-r1

DESCRIPTION="C++ runtime stack unwinder from LLVM"
HOMEPAGE="https://github.com/llvm-mirror/libunwind"
SRC_URI=""
EGIT_REPO_URI="${CROS_GIT_HOST_URL}/external/github.com/llvm/llvm-project
	${CROS_GIT_HOST_URL}/external/github.com/llvm/llvm-project"
EGIT_BRANCH=main

LICENSE="|| ( UoI-NCSA MIT )"
SLOT="0"
KEYWORDS="*"
IUSE="cros_host debug llvm-asserts +static-libs +shared-libs +synth_libgcc
	+compiler-rt"

# Disable unittests because they are slow and we test on-device.
# We can manually re-enable with accept-restrict if needed.
RESTRICT="test"

RDEPEND="!${CATEGORY}/libunwind"

DEPEND="
	${RDEPEND}
"

if [[ "${CATEGORY}" == cross-*-linux* ]]; then
	DEPEND+="
		${CATEGORY}/linux-headers
		${CATEGORY}/glibc
	"
elif [[ "${CATEGORY}" == cross-* ]]; then
	DEPEND+="
		${CATEGORY}/newlib
	"
else
	DEPEND+="
		sys-kernel/linux-headers
		sys-libs/glibc
	"
fi

BDEPEND="
	sys-devel/llvm
	${PYTHON_DEPS}
"

if [[ "${CATEGORY}" == cross-* ]]; then
	BDEPEND+="
		sys-libs/libcxx
	"
	# The x86 compiler-rt is provided by sys-devel/llvm.
	if [[ "${CATEGORY}" != cross-x86_64-* && "${CATEGORY}" != cross-i686-* ]]; then
		BDEPEND+="
			compiler-rt? ( ${CATEGORY}/compiler-rt )
		"
	fi
fi

if is_baremetal_abi && [[ ${CTARGET} == riscv* ]]; then
	export CROSTC_USER_ACKNOWLEDGES_THAT_RISCV_IS_EXPERIMENTAL=1
fi

pkg_setup() {
	# Setup llvm toolchain for cross-compilation
	setup_cross_toolchain
}

src_prepare() {
	python_setup

	cros-llvm_ensure_patches_applied

	export CMAKE_USE_DIR="${S}/runtimes"
	eapply_user
	cmake_src_prepare
}

should_enable_asserts() {
	if is_baremetal_abi; then
		echo no
	elif use debug || use llvm-asserts; then
		echo yes
	else
		echo no
	fi
}

multilib_src_configure() {
	# Disable sanitization of llvm-libunwind (b/193934733).
	use_sanitizers && filter_sanitizers

	# Filter default portage flags to allow unwinding.
	cros_enable_cxx_exceptions
	append-cppflags "-D_LIBUNWIND_USE_DLADDR=0"
	if [[ ${CATEGORY} == cross-armv7a* ]] ; then
		# Allow targeting non-neon targets for armv7a.
		append-flags -mfpu=vfpv3

		# cross-armv7 builds fail due a to libgcc_eh.a bootstrap bug
		# due to a global variable used for stack canary memory.
		# https://bugzilla.redhat.com/show_bug.cgi?id=708452
		# https://gcc.gnu.org/bugzilla/show_bug.cgi?id=102352
		# TODO(toolchain): Revisit this when the above GCC bug is
		# fixed or we have glibc arm clang build support.
		append-flags -fno-stack-protector
	fi

	if [[ $(should_enable_asserts) == no ]]; then
		append-cppflags "-DNDEBUG"
	fi

	local libdir=$(get_libdir)
	local mycmakeargs=(
		"${mycmakeargs[@]}"
		"-DCMAKE_TRY_COMPILE_TARGET_TYPE=STATIC_LIBRARY"
		"-DCMAKE_POSITION_INDEPENDENT_CODE=ON"
		"-DLLVM_LIBDIR_SUFFIX=${libdir#lib}"
		"-DLIBUNWIND_ENABLE_ASSERTIONS=$(should_enable_asserts)"
		"-DLIBUNWIND_ENABLE_STATIC=$(usex static-libs)"
		"-DLIBUNWIND_ENABLE_SHARED=OFF"
		"-DLIBUNWIND_ENABLE_THREADS=OFF"
		"-DLIBUNWIND_ENABLE_CROSS_UNWINDING=OFF"
		"-DLIBUNWIND_ENABLE_FRAME_APIS=ON"
		"-DLIBUNWIND_USE_COMPILER_RT=$(usex compiler-rt)"
		"-DLIBUNWIND_TARGET_TRIPLE=$(get_abi_CTARGET)"
		"-DCMAKE_INSTALL_PREFIX=${PREFIX}"
		"-DLLVM_ENABLE_RUNTIMES=libunwind"
		# Avoid old libstdc++ errors when bootstrapping.
		"-DLLVM_ENABLE_LIBCXX=ON"
		"-DLIBUNWIND_HAS_COMMENT_LIB_PRAGMA=OFF"
		"-DLIBUNWIND_HAS_DL_LIB=OFF"
		"-DLIBUNWIND_HAS_PTHREAD_LIB=OFF"
	)

	if is_baremetal_abi; then
		# Options for baremetal toolchains e.g. armv7m-cros-eabi.
		append-flags -Oz # Optimize for smallest size.
		mycmakeargs+=(
			"-DCMAKE_POSITION_INDEPENDENT_CODE=OFF"
			"-DLIBUNWIND_IS_BAREMETAL=ON"
			"-DLIBUNWIND_REMEMBER_HEAP_ALLOC=ON"
			"-DLLVM_ENABLE_LTO=Full"
		)

		# TODO(b/416568977): It would be more flexible to set up multilib so
		# that applications can customize the flags they want (CPU/arch,
		# soft/hard float, etc.)
		if [[ "${CTARGET}" == arm-none-eabi ]]; then
			# b/205342596: This is a hack to provide armv6m libraries for use with
			# arm-none-eabi without creating a separate armv6m toolchain.
			# shellcheck disable=SC2154 # CTARGET is set elsewhere
			append-flags "-march=armv6m --sysroot=/usr/arm-none-eabi"
			mycmakeargs+=(
				"-DCMAKE_C_COMPILER_TARGET=armv6m-none-eabi"
				"-DCMAKE_CXX_COMPILER_TARGET=armv6m-none-eabi"
			)
		elif [[ "${CTARGET}" == armv7m-cros-eabi ]]; then
			# b/286910996: Set target-specific floating point flags.
			append-flags -mcpu=cortex-m4
			append-flags -mfloat-abi=hard
		elif [[ ${CTARGET} == riscv* ]]; then
			# b/416568282: Set target-specific floating point flags.
			append-flags -march=rv32imfc
		fi
	fi

	cmake_src_configure
}

multilib_src_install_all() {
	# Remove files that are installed by sys-libs/llvm-libunwind
	# to avoid collision when installing cross-${TARGET}/llvm-libunwind.
	if [[ ${CATEGORY} == cross-* ]]; then
		rm -rf "${ED}"/usr/share || die
	fi

	# Install headers.
	insinto "${PREFIX}"/include
	doins -r "${S}"/libunwind/include/.
}

multilib_src_install() {
	cmake_src_install
	if is_baremetal_abi; then
		return
	fi

	# Generate libunwind.so or libgcc_s.so.
	local myabi=$(get_abi_CTARGET)
	if [[ ${myabi} == *armv7a* ]]; then
		LIBGCC_ARCH="armhf"
	elif [[ ${myabi} == *aarch64* ]]; then
		LIBGCC_ARCH="aarch64"
	elif [[ ${myabi} =~ ^i[0-9]86 ]]; then
		LIBGCC_ARCH="i386"
	elif [[ ${myabi} == *x86_64* ]] ; then
		LIBGCC_ARCH="x86_64"
	else
		echo "unsupported arch" && die
	fi

	local COMPILER_RT_BUILTINS=$($(tc-getCC) -print-libgcc-file-name -rtlib=compiler-rt)
	local my_installdir="${D%/}${PREFIX}/$(get_libdir)"
	local out_file soname

	if use synth_libgcc; then
		out_file=libgcc_s.so.1
		soname=libgcc_s.so.1
	else
		out_file=libunwind.so.1.0
		soname=libunwind.so.1
	fi

	echo "Creating ${out_file} using libunwind.a + compiler-rt".
	# Ignore split word warnings, we need them for flags.
	# shellcheck disable=SC2086
	$(tc-getCC) -o "${my_installdir}"/"${out_file}"                                 \
		${CFLAGS}                                                                   \
		${LDFLAGS}                                                                  \
		-shared                                                                     \
		-nostdlib                                                                   \
		-Wl,--whole-archive                                                         \
		-Wl,--version-script,"${FILESDIR}/version-scripts/gcc_s-${LIBGCC_ARCH}.ver" \
		-Wl,-soname,"${soname}"                                                     \
		"${COMPILER_RT_BUILTINS}"                                                   \
		"${my_installdir}"/libunwind.a                                              \
		-Wl,--no-whole-archive                                                      \
		-lm                                                                         \
		-lc                                                                         \
	|| die "Failed to create ${out_file}".

	# Point libunwind.so.1, libunwind_shared.so and libunwind.so to libunwind.so.1.0.
	ln -s libunwind.so.1.0 "${my_installdir}"/libunwind.so.1 || die
	ln -s libunwind.so.1   "${my_installdir}"/libunwind.so || die
	ln -s libunwind.so     "${my_installdir}"/libunwind_shared.so || die

	# Generate libgcc{,_eh,_s} if requested.
	if use synth_libgcc; then
		# We already created libgcc_s.so.1 if we are here.
		# Point libunwind.so.1.0 and libgcc_s.so to it.
		# Also make sure that libgcc.a and libgcc_eh.a point to compiler-rt/libunwind.
		ln -s libgcc_s.so.1 "${my_installdir}"/libunwind.so.1.0 || die
		ln -s libgcc_s.so.1 "${my_installdir}"/libgcc_s.so || die
		ln -s libunwind.a "${my_installdir}"/libgcc_eh.a || die
		cp "${COMPILER_RT_BUILTINS}" "${my_installdir}"/libgcc.a || die
	fi
}
