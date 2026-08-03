# Copyright 2022 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7
inherit cros-toolchain-funcs

GIT_HASH="bcf4b0dca8e58b8fc8265ba746a7b84af86fc365"
DESCRIPTION="Universal Flash Storage user space tooling for Linux"
HOMEPAGE="https://github.com/westerndigitalcorporation/ufs-utils"
SRC_URI="${HOMEPAGE}/archive/${GIT_HASH}.tar.gz -> ${P}.tar.gz"

LICENSE="BSD"
SLOT="0"
KEYWORDS="*"
IUSE=""

src_prepare() {
	default

	eapply "${FILESDIR}"/ufs-utils-1.10-Increase-chunk-size-for-ufs-ffu.patch

	eapply "${FILESDIR}"/ufs-utils-5.13.9-Fixup-header.patch

	eapply "${FILESDIR}"/ufs-utils-5.13.9-options-Fix-buffer-size-for-data-path.patch

	eapply "${FILESDIR}"/ufs-utils-6.14.11-Enable-Samsung-MX-FW.patch

	eapply "${FILESDIR}"/ufs-utils-7.14.12-ufs_cmds-Fix-size-filed-arrays.patch

	# Remove hard-coded version from Makefile, emake will set its own.
	sed -i -e 's/-D_FORTIFY_SOURCE=2//' Makefile || die
}

src_configure() {
	# Allow use of O_DIRECT
	append-cflags -D_GNU_SOURCE
}

src_compile() {
	emake CC="$(tc-getCC)"
}

src_install() {
	dosbin ufs-utils
}
