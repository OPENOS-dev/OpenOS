# Copyright 1999-2010 Gentoo Foundation
# Copyright 2023 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

inherit autotools arc-build-constants cros-sanitizers tmpfiles

DESCRIPTION="Ureadahead - Read files in advance during boot"
HOMEPAGE="https://github.com/rostedt/ureadahead"
GIT_HASH="e034cec088ad94e10407011ec17b42e7dc4721f1"
SRC_URI="https://github.com/rostedt/ureadahead/archive/${GIT_HASH}.tar.gz -> ${PN}-${GIT_HASH}.tar.gz"

LICENSE="GPL-2"
SLOT="0"
KEYWORDS="*"
IUSE="arcvm"

RDEPEND="
	sys-apps/util-linux
	>=sys-fs/e2fsprogs-1.41
	dev-libs/libtracefs
"
DEPEND="${RDEPEND}"
BDEPEND="
	sys-devel/gnuconfig
"

PATCHES=(
	"${FILESDIR}/pack_version_bump.patch"
)

src_unpack() {
	default
	mv "${PN}-${GIT_HASH}" "${P}"
}

src_configure() {
	sanitizers-setup-env
	econf --sbindir=/sbin
}

src_install() {
	default
	rm -r "${D}/etc/init"

	# install init script
	insinto /etc/init
	doins "${FILESDIR}"/init/*.conf

	dotmpfiles "${FILESDIR}"/tmpfiles.d/*

	# stage executable into guest vendor image for ARCVM to be installed into
	# system image via board_specific_setup.py
	if use arcvm; then
		arc-build-constants-configure
		# shellcheck disable=SC2154 # Defined in arc-build-constants.eclass.
		exeinto "${ARC_VM_VENDOR_DIR}/bin"
		doexe "${WORKDIR}/${P}/src/ureadahead"
	fi
}
