# Copyright 1999-2021 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

DESCRIPTION="Virtual for the C library"
SLOT="0"
KEYWORDS="*"

# explicitly depend on SLOT 2.2 of glibc, because it sets
# a different SLOT for cross-compiling
# Cygwin uses newlib, which lacks libcrypt
RDEPEND="!prefix-guest? (
		elibc_glibc? ( sys-libs/glibc:2.2 )
		elibc_musl? ( sys-libs/musl )
		elibc_uclibc? ( sys-libs/uclibc-ng )
	)
	prefix-guest? (
		elibc_Cygwin? ( sys-libs/cygwin-crypt )
		!sys-libs/glibc
		!sys-libs/musl
		!sys-libs/uclibc-ng
	)"
