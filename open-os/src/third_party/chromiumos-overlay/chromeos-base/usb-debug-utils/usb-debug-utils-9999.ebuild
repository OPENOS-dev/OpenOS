# Copyright 2024 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI="7"

CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_LOCALNAME="platform2"
CROS_WORKON_DESTDIR="${S}"
CROS_WORKON_OUTOFTREE_BUILD=1
CROS_WORKON_SUBTREE="usb-debug-utils"

inherit cros-workon

DESCRIPTION="Extra tools and scripts used when debugging USB."

LICENSE="BSD-Google"
SLOT="0"
KEYWORDS="~*"

src_install() {
	exeinto /usr/sbin/
	newexe usb-debug-utils/usb_debug_utils.sh usb_debug_utils
	newexe usb-debug-utils/usb_compliance_utils.sh usb_compliance_utils
}
