# Copyright 2023 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_WORKON_PROJECT="chromiumos/infra/build/empty-project"
CROS_WORKON_LOCALNAME="platform/empty-project"

inherit appid cros-workon

DESCRIPTION="Ebuild which pulls in any necessary ebuilds as dependencies
or portage actions."

LICENSE="BSD-Google"
SLOT="0"
KEYWORDS="~*"
IUSE="arm64_vm"

RDEPEND="
	arm64_vm? (
		chromeos-base/chromeos-gce-l1-vkms
		chromeos-base/chromeos-visl
		sys-boot/grub
	)
"
