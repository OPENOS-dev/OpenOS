# Copyright 2024 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_WORKON_PROJECT="chromiumos/infra/build/empty-project"
CROS_WORKON_LOCALNAME="../platform/empty-project"
CROS_WORKON_OUTOFTREE_BUILD=1

inherit cros-subtool cros-workon

DESCRIPTION="Subtools config for ti50-sdk package"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/overlays/chromiumos-overlay/+/HEAD/dev-embedded/ti50-sdk"

LICENSE="BSD-Google"
KEYWORDS="~*"

RDEPEND="dev-embedded/ti50-sdk"

src_install() {
	cros-subtool_src_install "${FILESDIR}/subtool.textproto"
}
