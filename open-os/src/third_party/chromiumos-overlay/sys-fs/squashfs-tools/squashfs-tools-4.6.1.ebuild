# Copyright 1999-2023 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

inherit flag-o-matic cros-toolchain-funcs cros-subtool

DESCRIPTION="Tools to create and extract Squashfs filesystems"
HOMEPAGE="https://github.com/plougher/squashfs-tools/"
SRC_URI="
	https://github.com/plougher/squashfs-tools/archive/${PV}.tar.gz
		-> ${P}.tar.gz
"
S=${WORKDIR}/${P}

LICENSE="GPL-2"
SLOT="0"
KEYWORDS="*"
IUSE="debug lz4 lzma lzo selinux xattr zstd"
REQUIRED_USE="selinux? ( xattr )"

DEPEND="
	sys-libs/zlib
	lz4? ( app-arch/lz4 )
	lzma? ( app-arch/xz-utils )
	lzo? ( dev-libs/lzo )
	xattr? (
		selinux? ( sys-libs/libselinux )
		sys-apps/attr
	)
	zstd? ( app-arch/zstd )
"
RDEPEND=${DEPEND}

src_prepare() {
	eapply "${FILESDIR}"/${P}-file-map.patch
	eapply "${FILESDIR}"/${P}-4k-align.patch
	eapply "${FILESDIR}"/${P}-selinux.patch
	eapply "${FILESDIR}"/${P}-regex.patch
	eapply_user
}

use10() {
	usex "${1}" 1 0
}

src_compile() {
	# set up make command line variables in EMAKE_SQUASHFS_CONF
	local opts=(
		LZMA_XZ_SUPPORT=$(use10 lzma)
		LZO_SUPPORT=$(use10 lzo)
		LZ4_SUPPORT=$(use10 lz4)
		SELINUX_SUPPORT=$(use10 selinux)
		XATTR_SUPPORT=$(use10 xattr)
		XZ_SUPPORT=$(use10 lzma)
		ZSTD_SUPPORT=$(use10 zstd)
	)

	tc-export CC
	use debug && append-cppflags -DSQUASHFS_TRACE
	emake "${opts[@]}" -C squashfs-tools
}

src_install() {
	dobin squashfs-tools/{mksquashfs,unsquashfs}
	dodoc ACKNOWLEDGEMENTS CHANGES README*
	doman manpages/*.1

	dosym unsquashfs /usr/bin/sqfscat
	dosym mksquashfs /usr/bin/sqfstar

	cros-subtool_src_install "${FILESDIR}/subtool.textproto"
}
