# Copyright 2024 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

inherit cros-sanitizers eutils autotools

DESCRIPTION="Epson Colorworks CUPS Filter"
HOMEPAGE="https://epson.com/Support/Printers/Label-Printers/ColorWorks-Series/sh/s131"
SRC_URI="
	gs://chromeos-localmirror/distfiles/epson-inkjet-printer-filter-1.0.1.tar.gz
	gs://chromeos-localmirror/distfiles/epson-inkjet-printer-cw-c4000-1.0.1.tar.gz
	gs://chromeos-localmirror/distfiles/epson-inkjet-printer-cw-c6000c6500-1.3.0.tar.gz
"

LICENSE="GPL-3"
SLOT="0"
KEYWORDS="*"
IUSE=""

DEPEND="
	net-print/cups:=
	media-libs/libpng:=
"
RDEPEND="${DEPEND}"

S="${WORKDIR}"

COMPONENTS=(
	epson-inkjet-printer-cw-c4000-1.0.1
	epson-inkjet-printer-cw-c6000c6500-1.3.0
	epson-inkjet-printer-filter-1.0.1
)

src_prepare() {
	default
	cros_enable_cxx_exceptions

	pushd epson-inkjet-printer-cw-c4000-1.0.1 || die
	eapply "${FILESDIR}/epson-inkjet-printer-cw-c4000-1.0.0-narrowing.patch"
	eapply "${FILESDIR}/epson-inkjet-printer-cw-c4000-1.0.0-libpath.patch"
	sed -i '/^CC=/d' configure.ac
	eautoreconf
	popd || die

	pushd epson-inkjet-printer-cw-c6000c6500-1.3.0 || die
	eapply "${FILESDIR}/epson-inkjet-printer-cw-c4000-1.0.0-narrowing.patch"
	eapply "${FILESDIR}/epson-inkjet-printer-cw-c4000-1.0.0-libpath.patch"
	sed -i '/^CC=/d' configure.ac
	eautoreconf
	popd || die

	pushd epson-inkjet-printer-filter-1.0.1 || die
	eapply "${FILESDIR}/epson-inkjet-printer-cw-c4000-1.0.0-libpath.patch"
	eapply "${FILESDIR}/epson-inkjet-printer-filter-1.0.1-name.patch"
	eapply "${FILESDIR}/epson-inkjet-printer-filter-1.0.1-ppd.patch"
	eautoreconf
	popd || die
}

src_configure() {
	sanitizers-setup-env
	append-lfs-flags

	for c in "${COMPONENTS[@]}"; do
		pushd "${c}" || die
		econf CUPS_SERVER_DIR=/usr/libexec/cups
		popd || die
	done
}

src_compile() {
	for c in "${COMPONENTS[@]}"; do
		pushd "${c}" || die
		emake
		popd || die
	done
}

src_install() {
	for c in "${COMPONENTS[@]}"; do
		pushd "${c}" || die
		emake -C src DESTDIR="${D}" install
		popd || die
	done
}
