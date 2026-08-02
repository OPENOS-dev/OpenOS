# Copyright 1999-2025 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

inherit cros-fuzzer cros-sanitizers eutils autotools

DESCRIPTION="Epson Inkjet Printer Driver (ESC/P-R)"
HOMEPAGE="http://download.ebz.epson.net/dsc/search/01/search/?OSC=LX"

# https://support.epson.net/linux/Printer/LSB_distribution_pages/en/escpr.php
# Use the "source package for arm CPU" to get a tarball instead of an srpm.
SRC_URI="gs://chromeos-localmirror/distfiles/${P}-1.tar.gz"

LICENSE="GPL-2"
SLOT="0"
KEYWORDS="*"
IUSE="fuzzer"

DEPEND="net-print/cups"
RDEPEND="${DEPEND}"

PATCHES=(
	"${FILESDIR}/${PV}-warnings.patch"
	"${FILESDIR}/${PN}-1.7.7-fnocommon.patch"
	"${FILESDIR}/${PN}-1.8-missing-include.patch"
	"${FILESDIR}/${PN}-1.8.6-cupsRasterHeader.patch"
	"${FILESDIR}/${PN}-1.7.25-lfs-support.patch"
	"${FILESDIR}/${PN}-1.8.6-asan-free-cups-options.patch"
	"${FILESDIR}/${PN}-1.8.6-Fix-the-calls-to-calloc.patch"
	"${FILESDIR}/${PN}-1.8.6-snprintf.patch"
	"${FILESDIR}/${PN}-1.8.6-linecount.patch"
	"${FILESDIR}/${PN}-1.8.6-checksize.patch"
)

src_prepare() {
	local f
	for f in $(find ./ -type f || die); do
		edos2unix "${f}"
	done

	default

	if use fuzzer ; then
		eapply "${FILESDIR}/${PN}-1.8.6-Add-fuzzer.patch"
		cp "${FILESDIR}"/epson_escpr_fuzzer.c "${FILESDIR}"/stdin_util.{h,c} src/
	fi

	eautoreconf
}

src_configure() {
	if use fuzzer ; then
		fuzzer-setup-env || die
	fi

	sanitizers-setup-env
	econf \
		--disable-shared \
		--with-cupsfilterdir=/usr/libexec/cups/filter \
		--with-cupsppddir=/usr/share/cups

	# Makefile calls ls to generate a file list which is included in Makefile.am
	# Set the collation to C to avoid automake being called automatically
	unset LC_ALL
	export LC_COLLATE=C
}

src_install() {
	emake -C src DESTDIR="${D}" install

	if use fuzzer ; then
		insinto /usr/share/cups/model
		doins "${FILESDIR}"/epson.ppd
		local fuzzer_component_id="167231"
		fuzzer_install "${FILESDIR}"/OWNERS.fuzzer src/epson_escpr_fuzzer --comp "${fuzzer_component_id}"
	fi
}
