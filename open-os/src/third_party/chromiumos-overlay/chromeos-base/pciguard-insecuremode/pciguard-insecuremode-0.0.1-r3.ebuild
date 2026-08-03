# Copyright 2024 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI="7"

CROS_WORKON_COMMIT="ebb8e32e609ba82b3bd540db54baa7c937216fb7"
CROS_WORKON_TREE="f144b5b46ca7658f88b814bc49132aa674f1b577"
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_LOCALNAME="platform2"
CROS_WORKON_DESTDIR="${S}"
CROS_WORKON_OUTOFTREE_BUILD=1
CROS_WORKON_SUBTREE="pciguard/insecure-mode"

inherit cros-workon

DESCRIPTION="Allow PCIGuard to be enabled/disabled on test images"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform2/+/HEAD/pciguard/insecure-mode"

LICENSE="BSD-Google"
SLOT="0"
KEYWORDS="*"

src_install() {
	insinto /etc/init
	doins pciguard/insecure-mode/*.{conf,override}

	insinto /lib/udev/rules.d
	doins pciguard/insecure-mode/81-thunderbolt.rules

	exeinto /usr/share/cros
	doexe pciguard/insecure-mode/pciguard_utils.sh

	exeinto /usr/share/cros/init
	doexe pciguard/insecure-mode/pciguard-insecuremode.sh
}
