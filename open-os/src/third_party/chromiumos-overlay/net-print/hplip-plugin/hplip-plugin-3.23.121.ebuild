# Copyright 1999-2023 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

inherit dlc unpacker

DESCRIPTION="Proprietary plugins and firmware for HPLIP"
HOMEPAGE="https://developers.hp.com/hp-linux-imaging-and-printing/plugins"
SRC_URI="gs://chromeos-localmirror/distfiles/hplip-${PV}-plugin.zip"
S="${WORKDIR}"

LICENSE="LICENSE.hplip-plugin"
SLOT="0"
KEYWORDS="*"
IUSE="dlc orblite"
REQUIRED_USE="dlc"

# TODO(b/324592980) - ignore the version number for now, the first line
#                     should be ~net-print/hplip-${PV}
RDEPEND="
	net-print/hplip:=
	virtual/udev
	orblite? (
		media-gfx/sane-backends
		>=sys-libs/glibc-2.26
		virtual/libusb:0
	)
"

BDEPEND="app-arch/unzip"

# Required by DLC
DLC_PREALLOC_BLOCKS="10240"
DLC_SCALED=true

HPLIP_HOME="$(dlc_add_path /)"

# Binary prebuilt package
QA_PREBUILT="${HPLIP_HOME}/*.so"

src_install() {
	local hplip_arch plugin
	case "${ARCH}" in
		amd64) hplip_arch="x86_64" ;;
		arm)   hplip_arch="arm32"  ;;
		arm64) hplip_arch="arm64"  ;;
		*)     die "Unsupported architecture." ;;
	esac

	insinto "${HPLIP_HOME}"
	insopts -m0755
	for plugin in lj-${hplip_arch}.so; do
		newins ${plugin} ${plugin/-${hplip_arch}}
	done

	dlc_src_install
}
