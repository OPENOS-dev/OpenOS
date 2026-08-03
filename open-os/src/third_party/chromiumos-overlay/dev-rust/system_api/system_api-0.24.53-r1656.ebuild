# Copyright 2019 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_WORKON_COMMIT="8ea3af2134687a3c1a13767edbf649412bd26e3f"
CROS_WORKON_TREE=("0ba423cae8238e4b5b3d178d3ef4ebd9a001c96a" "8290553e9693cf5efca9f3d75a8d81fd54db5cd2" "f42007e4a31278e678d51c10aeca399c103092d3" "808455434979e636c42c269d6f786406e2b2be86" "81b98a37471549e915e85559acff067cc6a9590d" "f744ada0b992faafd7a184874c69020cc84300b4" "da55b576c0a4e25a079599b4bde19f8651cf1d7e" "64d452aa9dc262e63c824de9d940066218b8ec44" "f5b9b06147bd01a8dee984b7bf2037b6a099c038" "66fc4e5f66284959a624a309d0667b9d6c3778ef" "1a2f71aa9a4e6651eea65cadd5fb84008f242c33" "3fad3ffa17926c5fd6a63889ff04354dc6a3d938" "c50c17149809b73c42e9bd8ad3ca0f52eb2b292f")
CROS_RUST_SUBDIR="system_api"

CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_LOCALNAME="../platform2"
CROS_WORKON_SUBTREE="${CROS_RUST_SUBDIR} cryptohome/dbus_bindings debugd/dbus_bindings dlcservice/dbus_adaptors login_manager/dbus_bindings shill/dbus_bindings spaced/dbus_bindings permission_broker/dbus_bindings power_manager/dbus_bindings printscanmgr/dbus_bindings swap_management/dbus_bindings vm_tools/dbus_bindings vtpm"

inherit cros-workon cros-rust cros-protobuf

DESCRIPTION="Chrome OS system API D-Bus bindings for Rust."
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform2/+/HEAD/system_api/"

LICENSE="BSD-Google"
SLOT="0/${PVR}"
KEYWORDS="*"

DEPEND="
	cros_host? ( dev-libs/protobuf:= )
	dev-rust/third-party-crates-src:=
	dev-rust/chromeos-dbus-bindings:=
	sys-apps/dbus:=
"
# (crbug.com/1182669): build-time only deps need to be in RDEPEND so they are pulled in when
# installing binpkgs since the full source tree is required to use the crate.
RDEPEND="${DEPEND}
	!chromeos-base/system_api-rust
"

BDEPEND="
	dev-rust/chromeos-dbus-bindings
"

src_install() {
	# We don't want the build.rs to get packaged with the crate. Otherwise
	# we will try and regenerate the bindings.
	rm build.rs || die "Cannot remove build.rs"

	cros-rust_src_install
}
