# Copyright 2024 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_WORKON_PROJECT="chromiumos/infra/build/empty-project"
CROS_WORKON_LOCALNAME="../platform/empty-project"
CROS_WORKON_OUTOFTREE_BUILD=1

inherit cros-subtool cros-workon

DESCRIPTION="Subtools definition to export pixz from app-arch/pixz."
HOMEPAGE="https://github.com/vasi/pixz"

LICENSE="BSD-2"
SLOT="0"
KEYWORDS="~*"
IUSE=""

DEPEND="app-arch/pixz"

src_install() {
	cros-subtool_src_install "${FILESDIR}/pixz_subtool.textproto"
}
