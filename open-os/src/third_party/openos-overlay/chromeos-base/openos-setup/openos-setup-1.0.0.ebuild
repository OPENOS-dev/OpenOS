# Copyright 2026 OCS (Open Code Studio)
# Distributed under the terms of the GNU General Public License v3

EAPI=7

DESCRIPTION="OPENOS First-Boot System Initialization — replaces ChromeOS OOBE with OPENOS setup"
HOMEPAGE="https://github.com/openeuler/OpenOS"
LICENSE="GPL-3"
SLOT="0"
KEYWORDS="amd64"
IUSE="fdroid opk"

DEPEND=""
RDEPEND=""

S="${WORKDIR}"

src_install() {
    # First-boot setup script
    exeinto /usr/libexec/openos
    doexe "${FILESDIR}/openos-first-boot.sh"

    # Upstart job — starts on boot-services milestone
    insinto /etc/init
    doins "${FILESDIR}/openos-first-boot.conf"
}

pkg_postinst() {
    # Ensure the completed flag is NOT present after fresh install,
    # so first-boot runs on next boot. Only remove it during image
    # build (when D is the build root). Skip on target device upgrades.
    if [ -n "${D}" ] && [ "${D}" != "/" ]; then
        # We are in image build chroot — remove any stale flag
        rm -f "${D}/mnt/stateful_partition/.openos_boot_completed" 2>/dev/null || true
    fi
}
