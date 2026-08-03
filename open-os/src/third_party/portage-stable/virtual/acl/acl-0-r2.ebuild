# Copyright 1999-2022 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

inherit multilib-build

DESCRIPTION="Virtual for acl support (sys/acl.h)"

LICENSE="metapackage"
SLOT="0"
KEYWORDS="*"
IUSE="static-libs"

RDEPEND="kernel_linux? ( >=sys-apps/acl-2.2.52-r1[static-libs?,${MULTILIB_USEDEP}] )"
