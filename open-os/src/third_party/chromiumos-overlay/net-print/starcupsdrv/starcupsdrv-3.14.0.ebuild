# Copyright 2017 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

inherit eutils cros-sanitizers

DESCRIPTION="CUPS filter and PPD files for Star Micronics printers"
HOMEPAGE="http://www.starmicronics.com"
SRC_URI="http://www.starmicronics.com/support/DriverFolder/drvr/starcupsdrv-${PV}_linux.tar.gz -> starcupsdrv-${PV}_linux.tar.gz"

LICENSE="GPL-2"
SLOT="0"
KEYWORDS="*"
IUSE=""

DEPEND="net-print/cups:="
RDEPEND="${DEPEND}"

PATCHES=(
	"${FILESDIR}"/${PN}-3.14.0-tsp143gt-array-access.patch
)

src_unpack() {
	default
	unpack ./Star_CUPS_Driver-${PV%_*}_linux/SourceCode/Star_CUPS_Driver-src-${PV%_*}.tar.gz
	mv Star_CUPS_Driver starcupsdrv-${PV} || die
}

src_configure() {
	sanitizers-setup-env
	append-lfs-flags
	default
}

src_install() {
	exeinto "$("${SYSROOT}"/usr/bin/cups-config --serverbin)/filter"
	doexe install/rastertostar
	doexe install/rastertostarm
	doexe install/rastertostarlm
}
