# Copyright 2023 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_WORKON_PROJECT="chromiumos/infra/build/empty-project"
CROS_WORKON_LOCALNAME="../platform/empty-project"
CROS_WORKON_OUTOFTREE_BUILD=1

inherit cros-subtool cros-workon

DESCRIPTION="Subtools definition to export qemu from app-emulation/qemu."
HOMEPAGE="https://www.qemu.org https://www.linux-kvm.org"

LICENSE="metapackage"
SLOT="0"
KEYWORDS="~*"
IUSE=""

# We depend on qemu rather than trying to host the textproto in the
# portage-stable overlay, or inherit cros-subtool in qemu.bashrc.
DEPEND="app-emulation/qemu"

src_install() {
	cros-subtool_src_install "${FILESDIR}/qemu_subtool.textproto"
}
