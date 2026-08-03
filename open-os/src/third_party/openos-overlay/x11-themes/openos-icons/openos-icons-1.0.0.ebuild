# Copyright 2026 OCS (Open Code Studio)
# License: GPL-3.0

EAPI=7

DESCRIPTION="OPENOS brand icons — light and dark logo variants"
HOMEPAGE="https://github.com/openeuler/OpenOS"
LICENSE="GPL-3"
SLOT="0"
KEYWORDS="amd64"

S="${WORKDIR}"

src_install() {
    insinto /usr/share/openos/icons
    doins "${FILESDIR}"/openos-logo.svg
    doins "${FILESDIR}"/openos-logo-light.svg
    doins "${FILESDIR}"/openos-logo-dark.svg
}
