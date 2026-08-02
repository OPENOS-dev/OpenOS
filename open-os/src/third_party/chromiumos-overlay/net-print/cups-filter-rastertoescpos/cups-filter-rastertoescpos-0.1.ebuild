# Copyright 2024 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI="7"

inherit flag-o-matic cros-toolchain-funcs

DESCRIPTION="Generic raster to ESC/POS CUPS filter for thermal receipt printers"
HOMEPAGE=""
SRC_URI="gs://chromeos-localmirror/distfiles/cups-filter-rastertoescpos-${PV}.tar.gz"

LICENSE="MIT"
SLOT="0"
KEYWORDS="*"
IUSE=""

S="${WORKDIR}/${PN}"

DEPEND="net-print/cups net-print/cups-filters"
RDEPEND="${DEPEND}"

src_configure() {
	append-lfs-flags
}

src_compile() {
	append-ldflags "-lcups -lcupsfilters"

	# We normally want FLAGS variables quoted, but when running the compiler
	# directly here, we want to let them expand.
	# shellcheck disable=SC2086
	$(tc-getCC) \
		-Wall -Wextra -Werror \
		${CFLAGS} \
		${CPPFLAGS} \
		${LDFLAGS} \
		${PN}.c \
		-o rastertoescpos \
		|| die "Failed to build rastertoescpos"
}

src_install() {
	exeinto "/usr/libexec/cups/filter"
	doexe "rastertoescpos"
}
