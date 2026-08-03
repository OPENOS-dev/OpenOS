# Copyright 2024 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_WORKON_PROJECT="chromiumos/infra/build/empty-project"
CROS_WORKON_LOCALNAME="../platform/empty-project"
CROS_WORKON_OUTOFTREE_BUILD=1

inherit cros-subtool cros-workon

DESCRIPTION="Subtools definition to export pv tool from sys-apps/pv."
HOMEPAGE="https://www.ivarch.com/programs/pv.shtml"

LICENSE="GPL-3+"
SLOT="0"
KEYWORDS="~*"
IUSE=""

DEPEND="sys-apps/pv"

src_install() {
	cros-subtool_src_install "${FILESDIR}/pv_subtool.textproto"
}
