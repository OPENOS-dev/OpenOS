# Copyright 2026 The OpenOS Authors
# Distributed under the terms of the GNU General Public License v3

EAPI=7

DESCRIPTION="Open-OS NOTHING UI Theme - Login & Shell styling"
HOMEPAGE="https://github.com/openeuler/OpenOS"
LICENSE="GPL-3"
SLOT="0"
KEYWORDS="amd64"

S="${WORKDIR}"

src_install() {
    # Theme CSS files
    insinto /usr/share/openos/theme
    doins "${FILESDIR}"/*.css

    # Symlink login CSS into chromeos-assets where Chrome picks up OOBE theme
    dosym /usr/share/openos/theme/login.css /usr/share/chromeos-assets/login/nothing-login.css
    dosym /usr/share/openos/theme/tokens.css /usr/share/chromeos-assets/login/nothing-tokens.css
    dosym /usr/share/openos/theme/shell.css /usr/share/chromeos-assets/shell/nothing-shell.css
    dosym /usr/share/openos/theme/webui.css /usr/share/chromeos-assets/webui/nothing-webui.css
}
