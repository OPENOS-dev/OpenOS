# Copyright 2024 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_WORKON_COMMIT="d2d95e8af89939f893b1443135497c1f5572aebc"
CROS_WORKON_TREE="776139a53bc86333de8672a51ed7879e75909ac9"
CROS_WORKON_PROJECT="chromiumos/infra/build/empty-project"
CROS_WORKON_LOCALNAME="../platform/empty-project"
CROS_WORKON_OUTOFTREE_BUILD=1

inherit cros-subtool cros-workon

DESCRIPTION="Subtools definition to export pv tool from sys-apps/pv."
HOMEPAGE="https://www.ivarch.com/programs/pv.shtml"

LICENSE="GPL-3+"
SLOT="0"
KEYWORDS="*"
IUSE=""

DEPEND="sys-apps/pv"

src_install() {
	cros-subtool_src_install "${FILESDIR}/pv_subtool.textproto"
}
