# Copyright 1999-2021 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

DESCRIPTION="Virtual for operating system headers"
SLOT="0"
KEYWORDS="*"

# depend on SLOT 0 of linux-headers, because kernel-2.eclass
# sets a different SLOT for cross-building
RDEPEND="
	!prefix-guest? (
		|| (
			kernel_linux? ( sys-kernel/linux-headers:0 )
			kernel_Winnt? (
				elibc_mingw? ( dev-util/mingw64-runtime )
			)
		)
	)
	prefix-guest? (
		!sys-kernel/linux-headers
	)"
