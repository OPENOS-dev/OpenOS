# Copyright 2024 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# @ECLASS: coreboot-sdk-build.eclass
# @BLURB: Common build implementation for coreboot-sdk compiler
# @DESCRIPTION:
# coreboot-sdk has a bootstrap compiler and builds for multiple architectures.
# This eclass has a common implementation for functions used across multiple
# packages for each compiler.

if [[ -z "${_ECLASS_COREBOOT_SDK_BUILD}" ]]; then
_ECLASS_COREBOOT_SDK_BUILD=1

# Check for EAPI 7+.
case "${EAPI:-0}" in
[0123456]) die "unsupported EAPI (${EAPI}) in eclass (${ECLASS})" ;;
esac

inherit cros-workon multiprocessing coreboot-sdk-versions

# The path the bootstrap compiler gets installed to.
COREBOOT_SDK_BOOTSTRAP_DESTDIR="/opt/coreboot-sdk-bootstrap"

# The path this package will install to.
: "${COREBOOT_SDK_DESTDIR:=/opt/coreboot-sdk}"

DEPEND="
	app-arch/zstd
	sys-libs/zlib
"
RDEPEND="
	!<dev-embedded/coreboot-sdk-0.0.1-r124
	${DEPEND}
"
BDEPEND="
	dev-lang/perl
	sys-devel/bison
	sys-devel/flex
	sys-devel/m4
	${DEPEND}
"

# URIs taken from buildgcc -u
# Needs to be synced with changes in the coreboot repo,
# then pruned to the minimum required set (e.g., no gdb, python, expat, llvm)
# shellcheck disable=SC2154
COREBOOT_SDK_SRC_URI="
	${GMP_SRC_URI}
	${MPFR_SRC_URI}
	${MPC_SRC_URI}
	${GCC_SRC_URI}
	${BINUTILS_SRC_URI}
"
: "${SRC_URI:=${COREBOOT_SDK_SRC_URI}}"

coreboot-sdk-build_src_unpack() {
	cros-workon_src_unpack
	S+="/util/crossgcc"
	cd "${S}" || die "Unable to cd to ${S}"
}

coreboot-sdk-build_src_prepare() {
	default
	rm -rf tarballs
	ln -s "${DISTDIR}" tarballs
}

coreboot-sdk-build_src_configure() {
	# Cross-compilers require IASL to be ready for buildgcc to be happy.
	PATH="${COREBOOT_SDK_DESTDIR}/bin:${PATH}"

	# We need the bootstrap compilers to build.
	PATH="${COREBOOT_SDK_BOOTSTRAP_DESTDIR}/bin:${PATH}"

	export PATH CC=gcc CXX=g++
}

_coreboot-sdk-build_fail() {
	local f
	for f in */.failed; do
		cat "${f%/*}/build.log"
	done
	die "buildgcc failed with arguments: $*"
}

# Run buildgcc.
coreboot-sdk-build_buildgcc() {
	./buildgcc -j "$(makeopts_jobs)" \
		--directory "${COREBOOT_SDK_DESTDIR}" \
		--destdir "${WORKDIR}" "$@" \
		|| _coreboot-sdk-build_fail "$@"

	# Cleanup excess junk and files that will cause collisions.
	rm -f "${WORKDIR}${COREBOOT_SDK_DESTDIR}"/.*.success
	rm -f "${WORKDIR}${COREBOOT_SDK_DESTDIR}"/share/buildgcc*
	rm -rf "${WORKDIR}${COREBOOT_SDK_DESTDIR}"/{info,share/info,share/man}
}

coreboot-sdk-build_src_install() {
	local files

	dodir "${COREBOOT_SDK_DESTDIR%/*}"
	cp -a "${WORKDIR}${COREBOOT_SDK_DESTDIR}" "${D}${COREBOOT_SDK_DESTDIR}" || die

	readarray -t files < <(find "${D}" -name '*.[ao]' -printf "/%P\n")
	dostrip -x "${files[@]}"
}

EXPORT_FUNCTIONS src_unpack src_prepare src_configure src_install

fi
