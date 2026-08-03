# Copyright 1999-2020 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI="7"

inherit flag-o-matic cros-toolchain-funcs cros-llvm

if [[ ${PV} == "9999" ]] ; then
	EGIT_REPO_URI="https://sourceware.org/git/newlib-cygwin.git"
	inherit git-r3
else
	SRC_URI="ftp://sourceware.org/pub/newlib/${P}.tar.gz"
	KEYWORDS="*"
fi

export CBUILD=${CBUILD:-${CHOST}}
export CTARGET=${CTARGET:-${CHOST}}
if [[ ${CTARGET} == "${CHOST}" ]] ; then
	if [[ ${CATEGORY} == cross-* ]] ; then
		export CTARGET=${CATEGORY#cross-}
	fi
fi

BDEPEND="
	sys-apps/texinfo
"

if [[ ${CATEGORY} == cross-* ]] ; then
	BDEPEND+="
		${CATEGORY}/binutils
	"
fi

DESCRIPTION="Newlib is a C library intended for use on embedded systems"
HOMEPAGE="https://sourceware.org/newlib/"

LICENSE="NEWLIB LIBGLOSS GPL-2"
SLOT="0"
IUSE="nls +threads unicode headers-only +nano"
RESTRICT="strip"

PATCHES=(
	"${FILESDIR}/${PN}-3.3.0-no-nano-cxx.patch"
)

NEWLIBBUILD="${WORKDIR}/build"
NEWLIBNANOBUILD="${WORKDIR}/build.nano"
NEWLIBNANOTMPINSTALL="${WORKDIR}/nano_tmp_install"

# Adding -U_FORTIFY_SOURCE to counter the effect of Gentoo's
# auto-addition of _FORTIFY_SOURCE at gcc site: bug #656018#c4
# Currently newlib can't be built itself when _FORTIFY_SOURCE
# is set.
CFLAGS_FULL="-ffunction-sections -fdata-sections -U_FORTIFY_SOURCE"
CFLAGS_NANO="-Os -ffunction-sections -fdata-sections -U_FORTIFY_SOURCE"

if is_baremetal_abi && [[ ${CTARGET} == riscv* ]]; then
	export CROSTC_USER_ACKNOWLEDGES_THAT_RISCV_IS_EXPERIMENTAL=1
fi

pkg_setup() {
	# Reject newlib-on-glibc type installs
	if [[ ${CTARGET} == "${CHOST}" ]] ; then
		case ${CHOST} in
			*-newlib|*-elf) ;;
			*) die "Use sys-devel/crossdev to build a newlib toolchain" ;;
		esac
	fi

	case ${CTARGET} in
		msp430*)
			if ver_test "$(gcc-version "${CTARGET}")" -lt 10.1; then
				# bug #717610
				die "gcc for ${CTARGET} has to be 10.1 or above"
			fi
			;;
	esac
}

src_prepare() {
	default

	if [[ ${CTARGET} == *-eabi ]]; then
		# b/261636413: We use __libc_{init,fini}_array for initialization, not
		# _init/_fini.
		eapply "${FILESDIR}"/0001-Set-have_init_fini-to-no-for-ARM.patch
		# b/254674977: These two patches are needed to build with clang for
		# ARMv7M/ARMv6.
		eapply "${FILESDIR}"/Disable-building-Linux-related-files-for-ARM-in-libg.patch
		eapply "${FILESDIR}"/Disable-linker-warning-that-uses-stabs.patch
	fi
}

src_configure() {
	# b/254674977: build with clang instead of gcc.
	if tc-is-clang; then
		export CC_FOR_TARGET=${CTARGET}-clang
		export CXX_FOR_TARGET=${CTARGET}-clang++
	fi

	if is_baremetal_abi; then
		# Note: compiling with -Oz impacts the performance of memcpy/memset, so
		# we're intentionally not using it here: b/405230727.

		# EC doesn't support unaligned access: b/239254184.
		if [[ ${CTARGET} == arm* ]]; then
			append-flags -mno-unaligned-access
		fi

		# TODO(b/416568977): It would be more flexible to set up multilib so
		# that applications can customize the flags they want (CPU/arch,
		# soft/hard float, etc.)
		if [[ ${CTARGET} == armv7m* ]]; then
			append-flags -mcpu=cortex-m4
			append-flags -mfloat-abi=hard
		elif [[ ${CTARGET} == riscv* ]]; then
			# b/416568282: Set target-specific floating point flags.
			append-flags -march=rv32imfc
			append-flags -mfloat-abi=hard
		fi
	fi

	# b/254531710: newlib compiles ASM files as well as C files, so whatever
	# flags were changed with append-flags need to made to CCASFLAGS as well.
	# append-flags does not include AS flags:
	# https://devmanual.gentoo.org/eclass-reference/flag-o-matic.eclass/index.html
	CCASFLAGS="${CFLAGS}"

	# TODO: we should fix this
	unset LDFLAGS
	# b/416568282: Save original flags before calling strip-unsupported-flags.
	CCASFLAGS_ORIG="${CCASFLAGS}"
	CFLAGS_ORIG="${CFLAGS}"
	CHOST=${CTARGET} strip-unsupported-flags

	local myconf=(
		# Disable legacy syscall stub code in newlib.  These have been
		# moved to libgloss for a long time now, so the code in newlib
		# itself just gets in the way.
		--disable-newlib-supplied-syscalls
	)

	# b/239063738: Enable 64-bit printf
	myconf+=( --enable-newlib-io-long-long )

	[[ ${CTARGET} == "spu" ]] \
		&& myconf+=( --disable-newlib-multithread ) \
		|| myconf+=( "$(use_enable threads newlib-multithread)" )

	# Enable retargetable locking for bare metal targets.
	# This option allows Zephyr and EC to provide locking implementation.
	if is_baremetal_abi; then
		myconf+=( "$(use_enable threads newlib-retargetable-locking)" )
	fi

	mkdir -p "${NEWLIBBUILD}"
	cd "${NEWLIBBUILD}" || die

	myconf+=(
		# We don't want the ./configure script to probe for ada support
		# since it invokes an unprefixed `gcc`
		acx_cv_cc_gcc_supports_ada=no
	)

	export "CFLAGS_FOR_TARGET=${CFLAGS_ORIG} ${CFLAGS_FULL}"
	export "CCASFLAGS=${CCASFLAGS_ORIG} ${CFLAGS_FULL}"
	ECONF_SOURCE=${S} \
	econf \
		"$(use_enable unicode newlib-mb)" \
		"$(use_enable nls)" \
		"${myconf[@]}"

	# Build newlib-nano beside newlib (original)
	# Based on https://tracker.debian.org/media/packages/n/newlib/rules-2.1.0%2Bgit20140818.1a8323b-2
	if use nano ; then
		mkdir -p "${NEWLIBNANOBUILD}" || die
		cd "${NEWLIBNANOBUILD}" || die
		export "CFLAGS_FOR_TARGET=${CFLAGS_ORIG} ${CFLAGS_NANO}"
		export "CCASFLAGS=${CCASFLAGS_ORIG} ${CFLAGS_NANO}"
		ECONF_SOURCE=${S} \
		econf \
			"$(use_enable unicode newlib-mb)" \
			"$(use_enable nls)" \
			--enable-newlib-reent-small \
			--disable-newlib-fvwrite-in-streamio \
			--disable-newlib-fseek-optimization \
			--disable-newlib-wide-orient \
			--enable-newlib-nano-malloc \
			--disable-newlib-unbuf-stream-opt \
			--enable-lite-exit \
			--enable-newlib-global-atexit \
			--enable-newlib-nano-formatted-io \
			"${myconf[@]}"
	fi
}

src_compile() {
	export "CFLAGS_FOR_TARGET=${CFLAGS_ORIG} ${CFLAGS_FULL}"
	export "CCASFLAGS=${CCASFLAGS_ORIG} ${CFLAGS_FULL}"
	emake -C "${NEWLIBBUILD}"

	if use nano ; then
		export "CFLAGS_FOR_TARGET=${CFLAGS_ORIG} ${CFLAGS_NANO}"
		export "CCASFLAGS=${CCASFLAGS_ORIG} ${CFLAGS_NANO}"
		emake -C "${NEWLIBNANOBUILD}"
	fi
}

src_install() {
	cd "${NEWLIBBUILD}" || die
	emake -j1 DESTDIR="${D}" install

	if use nano ; then
		cd "${NEWLIBNANOBUILD}" || die
		emake -j1 DESTDIR="${NEWLIBNANOTMPINSTALL}" install
		# Rename nano lib* files to lib*_nano and move to the real ${D}
		local nanolibfiles=""
		nanolibfiles=$(find "${NEWLIBNANOTMPINSTALL}" -regex ".*/lib\(c\|g\|rdimon\)\.a" -print)
		for f in ${nanolibfiles}; do
			local l="${f##"${NEWLIBNANOTMPINSTALL}"}"
			mv -v "${f}" "${D}/${l%%\.a}_nano.a" || die
		done

		# Move newlib-nano's version of newlib.h to newlib-nano/newlib.h
		mkdir -p "${D}/usr/${CTARGET}/include/newlib-nano" || die
		mv "${NEWLIBNANOTMPINSTALL}/usr/${CTARGET}/include/newlib.h" \
			"${D}/usr/${CTARGET}/include/newlib-nano/newlib.h" || die
	fi

	# minor hack to keep things clean
	rm -rf "${D}"/usr/share/info || die
	rm -rf "${D}"/usr/info || die
}
