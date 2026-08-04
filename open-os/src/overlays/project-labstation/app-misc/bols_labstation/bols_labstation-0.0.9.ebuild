# Copyright 2025 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

# To determine the latest version, run the following for each architecture:
# $ cipd resolve chromiumos/infra/cft/bols_labstation/linux-${ARCH} -version latest

DESCRIPTION="BOLS for ChromeOS Labstation"
HOMEPAGE="https://chromium.googlesource.com/infra/infra/+/refs/heads/main/go/src/infra/cros/cmd/cft/bols_labstation"
SRC_URI="
	amd64? ( cipd://chromiumos/infra/cft/bols_labstation/linux-amd64:ddQT1Ub0gqyJsi9OInLB1cWlKgHOtgPYaY6b6nvwq7UC -> ${P}-amd64.zip )
"
RESTRICT="mirror"

LICENSE="BSD-Google"
SLOT="0"
KEYWORDS="*"
IUSE=""

BDEPEND="app-arch/unzip"

S="${WORKDIR}"

src_install() {
	into "/usr/local/"
	dobin bols_labstation
}
