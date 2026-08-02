# Copyright 2026 The OpenOS Authors
# Distributed under the terms of the GNU General Public License v3

EAPI=7

DESCRIPTION="Open-OS Meta Package - Base system configuration"
HOMEPAGE="https://github.com/openeuler/OpenOS"
LICENSE="GPL-3"
SLOT="0"
KEYWORDS="amd64"
IUSE="anopendroid +fdroid opk +openos-branding"

RDEPEND="
    chromeos-base/openos-setup
    chromeos-base/openos-theme
    anopendroid? ( anopendroid/anopendroid-runtime )
    fdroid? ( anopendroid/fdroid-privileged-extension )
    opk? ( app-admin/opk )
    openos-branding? (
        x11-themes/openos-wallpapers
        x11-themes/openos-cursor
        x11-themes/openos-icons
    )
"

S="${WORKDIR}"

src_install() {
    # Meta package — no files to install, just dependency binding.
    :;
}
