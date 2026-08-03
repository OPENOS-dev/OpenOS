# Copyright 2012 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI="7"
CROS_WORKON_COMMIT="a5357323e05a7893a72bb07a957d2ca5d0f9c80a"
CROS_WORKON_TREE="65c3079ab0e502da0b6fc4d99e1a861b00a16395"
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_LOCALNAME="platform2"
CROS_WORKON_DESTDIR="${S}"
CROS_WORKON_OUTOFTREE_BUILD=1
CROS_WORKON_SUBTREE="init/upstart/test-init"

inherit cros-workon

DESCRIPTION="Additional upstart jobs that will be installed on test images"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform2/+/HEAD/init/"

LICENSE="BSD-Google"
SLOT="0"
KEYWORDS="*"
IUSE=""

# File cryptohome-dbus-perf.conf moved from hwsec-test-utils.
RDEPEND="!<chromeos-base/hwsec-test-utils-0.0.1-r83"

src_unpack() {
	cros-workon_src_unpack
	S+="/init"
}

src_install() {
	insinto /etc/init
	doins upstart/test-init/*.conf

	insinto /usr/share/cros
	doins upstart/test-init/factory_utils.sh
}
