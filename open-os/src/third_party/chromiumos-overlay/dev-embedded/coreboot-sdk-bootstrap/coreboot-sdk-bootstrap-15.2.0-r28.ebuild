# Copyright 2024 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

EAPI=7

CROS_WORKON_COMMIT="82390cc046aaa9e947f1e5261269204c6ea4122d"
CROS_WORKON_TREE="bc79451ecc77e95c4a0ef3b049d10b4355d09b84"
CROS_WORKON_PROJECT="chromiumos/third_party/coreboot"
CROS_WORKON_LOCALNAME="coreboot"
CROS_WORKON_SUBTREE="util/crossgcc"

inherit cros-workon coreboot-sdk-build coreboot-sdk-versions

COREBOOT_SDK_DESTDIR="${COREBOOT_SDK_BOOTSTRAP_DESTDIR:-}"
DESCRIPTION="bootstrap compiler for coreboot-sdk"
HOMEPAGE="https://www.coreboot.org"
LICENSE="GPL-3 LGPL-3"
KEYWORDS="*"

SRC_URI="
	${COREBOOT_SDK_SRC_URI:-}
	http://mirrors.cdn.adacore.com/art/591c6d80c7a447af2deed1d7 -> gnat-gpl-2017-x86_64-linux-bin.tar.gz
"

src_unpack() {
	coreboot-sdk-build_src_unpack
	unpack gnat-gpl-2017-x86_64-linux-bin.tar.gz

	# buildgcc uses 'cc' to find gnat1 so it needs to find the gnat-gpl
	# compiler under that name
	ln -s gcc gnat-gpl-2017-x86_64-linux-bin/bin/cc
}

src_compile() {
	# To bootstrap the Ada build, an Ada compiler needs to be available. To
	# make sure it interacts well with the C/C++ parts of the compiler,
	# buildgcc asks gcc for the Ada compiler's path using the compiler's
	# -print-prog-name option which only deals with programs from the very
	# same compiler distribution, so make sure we use the right one.
	(
		export PATH="${S}/gnat-gpl-2017-x86_64-linux-bin/bin:${PATH}"

		coreboot-sdk-build_buildgcc --languages c,c++,ada --bootstrap-only
	)

	(
		set -x
		tc-export BUILD_CC
		mkdir build-BINUTILS || die "mkdir build-BINUTILS"
		cd build-BINUTILS || die "cd build-BINUTILS"
		export CC="${WORKDIR}${COREBOOT_SDK_DESTDIR}"/bin/x86_64-pc-linux-gnu-gcc
		export GCC="${CC}"
		export CC_FOR_TARGET="${CC}"
		export GCC_FOR_TARGET="${CC}"
		export CXX="${WORKDIR}${COREBOOT_SDK_DESTDIR}"/bin/x86_64-pc-linux-gnu-g++
		# BINUTILS_VERSION comes from coreboot-sdk-versions
		# shellcheck disable=SC2154
		"../binutils-${BINUTILS_VERSION}/configure" \
			--prefix="${COREBOOT_SDK_DESTDIR}" \
			--host=x86_64-pc-linux-gnu --verbose \
			--disable-werror \
			--disable-nls --enable-lto --enable-gold --enable-multilib \
			--disable-docs --disable-texinfo || die "configure failed"
		emake -j "$(makeopts_jobs)" || die "make failed"
		emake install DESTDIR="${WORKDIR}" || die "make install failed"
	)
}

src_install() {
	coreboot-sdk-build_src_install

	# Building target compilers will look for unprefixed cc in path.
	# Add a symlink so it goes to the bootstrap compiler.
	dosym gcc "${COREBOOT_SDK_DESTDIR:?}/bin/cc"

	# Only some of the binaries have the x86_64-pc-linux-gnu- prefix, create links for the others
	(
		cd "${D:?}${COREBOOT_SDK_DESTDIR:?}/bin" || die "Failed to cd to destdir"
		for i in *
		do
			if ! [[ "${i}" == x86_64-pc-linux-gnu-* ]] ; then
				if ! [ -e "${COREBOOT_SDK_DESTDIR:?}/bin/x86_64-pc-linux-gnu-${i}" ] ; then
					dosym "${i}" "${COREBOOT_SDK_DESTDIR:?}/bin/x86_64-pc-linux-gnu-${i}"
				fi
			fi
		done
	)
}
