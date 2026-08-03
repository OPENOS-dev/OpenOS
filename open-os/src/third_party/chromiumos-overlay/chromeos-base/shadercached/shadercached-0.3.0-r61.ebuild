# Copyright 2022 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_WORKON_COMMIT="39292427fcc86cc4758c7940c2b0d148e23739b7"
CROS_WORKON_TREE="a8269e8f8c1bd0bc3dfe52ba503d8062f9b6f474"
CROS_WORKON_INCREMENTAL_BUILD=1
CROS_WORKON_LOCALNAME="platform2"
CROS_WORKON_PROJECT="chromiumos/platform2"
# We don't use CROS_WORKON_OUTOFTREE_BUILD here since project's Cargo.toml is
# using "provided by ebuild" macro which supported by cros-rust.
CROS_WORKON_SUBTREE="shadercached"

inherit cros-workon cros-rust user

DESCRIPTION="Shader cache management daemon"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform2/+/HEAD/shadercached/"

LICENSE="BSD-Google"
SLOT="0/${PVR}"
KEYWORDS="*"

DEPEND="
	dev-rust/third-party-crates-src:=
	dev-rust/system_api:=
	dev-rust/libchromeos:=
	dev-libs/openssl:0=
	sys-apps/dbus:=
"
RDEPEND="sys-apps/dbus:="

src_install() {
	dobin "$(cros-rust_get_build_dir)/shadercached"

	# create a directory in /etc so that /run/daemon-store is created and mounted
	# by cryptohome
	local daemon_store="/etc/daemon-store/shadercached"
	dodir "${daemon_store}"
	fperms 0750 "${daemon_store}"
	fowners shadercached:shadercached "${daemon_store}"

	# D-Bus configuration.
	insinto /etc/dbus-1/system.d
	doins dbus/org.chromium.ShaderCache.conf

	# Init configuration
	insinto /etc/init
	doins init/shadercached.conf

	# Minijail configuration.
	insinto /usr/share/minijail
	doins minijail/shadercached.conf

	# Safesetid configuration.
	insinto /usr/share/cros/startup/process_management_policies
	doins setuid_restrictions/shadercached_allowlist.txt
}

src_test() {
	# Single threaded test execution to reduce test flakiness
	cros-rust_src_test -- --test-threads=1
}

pkg_setup() {
	# enewuser/group has to be done in pkg_setup() instead of pkg_preinst() since
	# src_install() needs shadercached user and group
	enewuser shadercached
	enewgroup shadercached
	# Note that cros-rust_pkg_setup calls cros-workon_pkg_setup for us.
	cros-rust_pkg_setup
}
