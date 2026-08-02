# Copyright 2024 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_WORKON_COMMIT="d2d95e8af89939f893b1443135497c1f5572aebc"
CROS_WORKON_TREE="776139a53bc86333de8672a51ed7879e75909ac9"
CROS_WORKON_PROJECT="chromiumos/infra/build/empty-project"
CROS_WORKON_LOCALNAME="../platform/empty-project"
CROS_WORKON_OUTOFTREE_BUILD=1

inherit cros-subtool cros-workon

DESCRIPTION="Subtools config for ti50-sdk package"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/overlays/chromiumos-overlay/+/HEAD/dev-embedded/ti50-sdk"

LICENSE="BSD-Google"
KEYWORDS="*"

RDEPEND="dev-embedded/ti50-sdk"

src_install() {
	cros-subtool_src_install "${FILESDIR}/subtool.textproto"
}
