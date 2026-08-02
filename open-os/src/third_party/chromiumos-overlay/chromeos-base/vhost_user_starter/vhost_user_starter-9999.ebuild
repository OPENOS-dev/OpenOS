# Copyright 2024 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_WORKON_INCREMENTAL_BUILD=1
CROS_WORKON_LOCALNAME="platform2"
CROS_WORKON_PROJECT="chromiumos/platform2"
# We don't use CROS_WORKON_OUTOFTREE_BUILD here since project's Cargo.toml is
# using "provided by ebuild" macro which supported by cros-rust.
PLATFORM2_PATHS=(
	common-mk
	.gn

	vm_tools/vhost_user_starter
	vm_tools/dbus_bindings
)

CROS_WORKON_SUBTREE="${PLATFORM2_PATHS[*]}"

CROS_RUST_SUBDIR="vm_tools/vhost_user_starter"

inherit cros-workon cros-rust cros-protobuf user

DESCRIPTION="ChromeOS VirtioVhostUserDevice Starter Daemon"

LICENSE="BSD-Google"
SLOT="0/${PVR}"
KEYWORDS="~*"

COMMON_DEPEND="
	sys-apps/dbus:=
"

DEPEND="
	${COMMON_DEPEND}
	chromeos-base/system_api:=
	dev-rust/libchromeos:=
	dev-rust/system_api:=
"

BDEPEND="
	chromeos-base/chromeos-dbus-bindings
"

RDEPEND="${COMMON_DEPEND}"

src_install() {
	# platform_src_install
	# cargo doesn't know how to install cross-compiled binaries. It will
	# always install native binaries for the host system.  Manually install
	# vhost_user_starter instead.
	local build_dir="$(cros-rust_get_build_dir)"
	dobin "${build_dir}/vhost_user_starter"

	# D-Bus configuration.
	insinto /etc/dbus-1/system.d
	doins dbus/org.chromium.VhostUserStarter.conf

	# init script.
	insinto /etc/init
	doins init/vhost_user_starter.conf

	# Minijail configuration.
	insinto /usr/share/minijail
	doins minijail/vhost_user_starter.conf

}

src_test() {
	cros-rust_src_test
	(cd "${S}" && ecargo fmt --check) || die "formatting is broken!"
}
