# Copyright 2024 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

inherit eutils cros-sanitizers cros-toolchain-funcs

DESCRIPTION="CUPS filter and PPD files for Hwasung printers"
HOMEPAGE="http://en.hwasungt.co.kr/"
SRC_URI="https://commondatastorage.googleapis.com/chromeos-localmirror/distfiles/hwasung-cupsdrv-${PV}.tar.gz"

LICENSE="Apache-2.0"
SLOT="0"
KEYWORDS="*"
IUSE=""

DEPEND="net-print/cups"
RDEPEND="${DEPEND}"

S="${WORKDIR}/source"
PATCHES=(
	"${FILESDIR}/${PN}-1.0.0-fix-fn-declarations.patch"
)

src_configure() {
	sanitizers-setup-env
	append-lfs-flags
	default
}

src_compile() {
	local CC="$(tc-getCC) $(get_abi_CFLAGS)"
	local CFLAGS="${CFLAGS} -Wall -fPIC -O2"
	local CPPFLAGS="${CPPFLAGS}"
	local LDFLAGS="${LDFLAGS} $(get_abi_LDFLAGS) -Wl,-rpath,/usr/lib -lcupsimage -lcups"
	# Splitting flags is intentional.
	# shellcheck disable=SC2086
	${CC} ${CFLAGS} ${CPPFLAGS} -o rastertohwasung rastertohwasung.c ${LDFLAGS}
}

src_install() {
	exeinto "/usr/libexec/cups/filter"
	doexe "${S}/rastertohwasung"
}
