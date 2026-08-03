# Copyright 2024 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7
CROS_WORKON_COMMIT="06374e4dc1d10081ddc71b456513716184545bc1"
CROS_WORKON_TREE="d1b7291bc26bf80c44bf14b8658c566acfd3f374"
CROS_WORKON_LOCALNAME="platform2"
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_SUBTREE="mini_udisks"
CROS_WORKON_INCREMENTAL_BUILD="1"

inherit cros-workon cros-rust tmpfiles

LICENSE="BSD-Google"
SLOT="0/${PVR}"
KEYWORDS="*"

DEPEND="
	dev-rust/third-party-crates-src:=
	dev-rust/libchromeos:=
	sys-apps/dbus:=
"
RDEPEND="
	${DEPEND}
	acct-group/mini_udisks
	acct-user/mini_udisks
"

src_install() {
	dotmpfiles tmpfiles.d/mini_udisks.conf

	insinto /etc/dbus-1/system.d
	doins dbus/org.freedesktop.UDisks2.conf

	insinto /usr/share/minijail
	doins minijail/mini_udisks.conf

	insinto /usr/share/policy
	doins minijail/mini_udisks-seccomp.policy

	insinto /etc/init
	doins upstart/mini_udisks.conf

	dobin "$(cros-rust_get_build_dir)/mini_udisks"
}
