# Copyright 2020-2022 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

inherit multilib-build

DESCRIPTION="Virtual for libcrypt.so"

SLOT="0/2"
KEYWORDS="*"
IUSE="static-libs"
LICENSE="metapackage"

RDEPEND="
	!prefix-guest? (
		elibc_glibc? ( sys-libs/libxcrypt[system(-),static-libs(-)?,${MULTILIB_USEDEP}] )
		elibc_musl? ( sys-libs/musl )
	)
	elibc_Cygwin? ( sys-libs/cygwin-crypt )
"
