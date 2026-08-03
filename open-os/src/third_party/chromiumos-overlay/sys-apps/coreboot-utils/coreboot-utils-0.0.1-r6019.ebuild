# Copyright 2012 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2
# $Header:

EAPI=7
CROS_WORKON_COMMIT=("33728a9e289605597efb84cc410113e2400852ac" "906136da2c93e6dfe3c93763c93dfae59d24e54d")
CROS_WORKON_TREE=("0589ed8c9f4da839ddab05f1ac1b302b3528c1d4" "d393877053b416c12909fec869a5dd3b85f4fe6a" "a3603a4550c49f66d47276dd0a9abd93d9af7028" "3c4c2d1f78d70547ae755c56c69f969581b0b975" "276cf8939b5dd5d5ad9bfaf3723e9145e3bdd713" "9b6631d3f3d0a39dadeafdafacdd5c4cefc910f5" "bc79451ecc77e95c4a0ef3b049d10b4355d09b84" "9257d2b8228647861d9f98fe9cb3114be232fe2c" "5666704fad52e38634f8909978a408e6dc94d8a4" "007bbccdbe3a9583d5b3edd1cb3dab919df41d4f" "7aaf9a340617c37a80f6d12f368730bc3c28cee4" "61e9fb595d3fbd38a3981026c91a86af76fc0b4c" "b96750709013c96b1afae05d15e61d8f608b1481" "ef4cae279a4da75f4f982873b1d2ca4e3d75b205" "456eac40a9371b7c5f2cd7345f0eb6df4d5433c2" "c51fe17f9dc332b4160ae4072e5ddffbdcc51737" "d509ef449a0f7cc8a53159eb5cb9b9a3e4761f8d" "3fef97a01c8310e42e8d39880bdbd2a6f1da0cbf")
CROS_WORKON_PROJECT=(
	"chromiumos/third_party/coreboot"
	"chromiumos/platform/vboot_reference"
)
CROS_WORKON_LOCALNAME=(
	"coreboot"
	"../platform/vboot_reference"
)
CROS_WORKON_DESTDIR=(
	"${S}"
	"${S}/3rdparty/vboot"
)
CROS_WORKON_EGIT_BRANCH=(
	"main"
	"main"
)

# coreboot:src/arch/x85/include/arch: used by inteltool, x86 only
# coreboot:src/commonlib: used by cbfstool
# coreboot:src/vendorcode/intel: used by cbfstool
# coreboot:util/*: tools built by this ebuild
# vboot: minimum set of files and directories to build vboot_lib for cbfstool
CROS_WORKON_SUBTREE=(
	"src/arch/x86/include/arch src/commonlib src/vendorcode/intel util/archive util/cbmem util/cbfstool util/crossgcc util/ifdtool util/inteltool util/mma util/nvramtool util/superiotool util/amdfwtool"
	"Makefile cgpt host firmware futility"
)

inherit cros-subtool cros-workon cros-toolchain-funcs cros-sanitizers

DESCRIPTION="Utilities for modifying coreboot firmware images"
HOMEPAGE="http://coreboot.org"
LICENSE="GPL-2"
SLOT="0"
KEYWORDS="*"
IUSE="cros_host mma +pci static"

BDEPEND="virtual/pkgconfig"

LIB_DEPEND="
	sys-apps/pciutils[static-libs(+)]
	sys-apps/flashrom
"
RDEPEND="!static? ( ${LIB_DEPEND//\[static-libs(+)]} )"
DEPEND="${RDEPEND}
	static? ( ${LIB_DEPEND} )
"

_emake() {
	emake \
		TOOLLDFLAGS="${LDFLAGS}" \
		CC="${CC}" \
		STRIP="true" \
		"$@"
}

src_configure() {
	sanitizers-setup-env
	use static && append-ldflags -static
	tc-export CC PKG_CONFIG
}

is_x86() {
	use x86 || use amd64
}

src_compile() {
	_emake -C util/cbfstool obj="${PWD}/util/cbfstool"
	if use cros_host; then
		_emake -C util/archive HOSTCC="${CC}"
	else
		_emake -C util/cbmem
	fi
	if is_x86; then
		_emake -C util/ifdtool
		if use cros_host; then
			_emake -C util/amdfwtool
		else
			_emake -C util/superiotool \
				CONFIG_PCI=$(usex pci)
			_emake -C util/inteltool
			_emake -C util/nvramtool
		fi
	fi
}

src_install() {
	dobin util/cbfstool/cbfstool
	dobin util/cbfstool/elogtool
	if use cros_host; then
		cros-subtool_src_install "${FILESDIR}"/*_subtool.textproto
		dobin util/cbfstool/fmaptool
		dobin util/cbfstool/cbfs-compression-tool
		dobin util/archive/archive
	else
		dobin util/cbmem/cbmem
	fi
	if is_x86; then
		dobin util/ifdtool/ifdtool
		if use cros_host; then
			dobin util/amdfwtool/amdfwread
		else
			dobin util/superiotool/superiotool
			dobin util/inteltool/inteltool
			dobin util/nvramtool/nvramtool
		fi
		if use mma; then
			dobin util/mma/mma_setup_test.sh
			dobin util/mma/mma_get_result.sh
			dobin util/mma/mma_automated_test.sh
			insinto /etc/init
			doins util/mma/mma.conf
		fi
	fi
}

src_test() {
	echo "Checking coreboot-sdk-versions.eclass file"
	"/mnt/host/source/src/third_party/chromiumos-overlay/eclass/gen/coreboot_sdk_versions.sh" --diff "${S}/util/crossgcc/buildgcc" "/mnt/host/source/src/third_party/chromiumos-overlay/eclass/coreboot-sdk-versions.eclass" || die
}
