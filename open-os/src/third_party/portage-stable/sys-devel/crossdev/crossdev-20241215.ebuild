# Copyright 1999-2025 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI="7"

if [[ ${PV} == "99999999" ]] ; then
	inherit git-r3
	EGIT_REPO_URI="
		https://anongit.gentoo.org/git/proj/crossdev.git
		https://github.com/gentoo/crossdev
	"
else
	SRC_URI="https://dev.gentoo.org/~sam/distfiles/${CATEGORY}/${PN}/${P}.tar.xz"
	KEYWORDS="*"
fi

DESCRIPTION="Gentoo Cross-toolchain generator"
HOMEPAGE="https://wiki.gentoo.org/wiki/Project:Crossdev"

LICENSE="GPL-2"
SLOT="0"

RDEPEND="
	>=sys-apps/portage-2.1
	app-shells/bash
	sys-apps/gentoo-functions
"
BDEPEND="app-arch/xz-utils"

# once we upgrade past an upstream version containing these last patches, we can
# move this ebuild back to portage-stable
PATCHES=(
	"${FILESDIR}/0001-cross-pkg-config-call-CBUILD-pkg-config-when-availab.patch"
	"${FILESDIR}/0002-crossdev-allow-ex-pkg-to-install-stable.patch"
)

src_install() {
	default

	if [[ "${PV}" == "99999999" ]] ; then
		sed -i "s:@CDEVPV@:${EGIT_VERSION}:" "${ED}"/usr/bin/crossdev || die
	fi
}
