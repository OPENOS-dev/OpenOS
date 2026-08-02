# Copyright 2024 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

inherit eutils cros-sanitizers cros-toolchain-funcs autotools

GIT_SHA="11674df159db6fb807ba06f6f19d6baad947e5ef"
DESCRIPTION="CUPS filter and PPD files for TSC printers"
HOMEPAGE="https://usca.tscprinters.com/en"
SRC_URI="https://github.com/zwlkevin/symmetrical-octo-journey/archive/${GIT_SHA}.tar.gz -> tsc_cups_driver-${PV}.tar.gz"

LICENSE="Apache-2.0"
SLOT="0"
KEYWORDS="*"
IUSE=""

DEPEND="net-print/cups"
RDEPEND="${DEPEND}"

S="${WORKDIR}/symmetrical-octo-journey-${GIT_SHA}"
PATCHES=(
	"${FILESDIR}/${PN}-1.0.0-remove-GTK-check.patch"
	"${FILESDIR}/${PN}-1.0.0-Don-t-strip-binary.patch"
	"${FILESDIR}/${PN}-1.0.0-add-arm64-support-for-loading-libraries.patch"
)

src_configure() {
	eautoreconf -ivf || die
	sanitizers-setup-env
	append-lfs-flags
	default
}

src_compile() {
	emake || die
}

src_install() {
	exeinto "/usr/libexec/cups/filter"
	doexe "${S}"/src/rastertobarcodetspl
}
