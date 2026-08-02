# Copyright 2018 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI="7"
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_LOCALNAME="platform2"
CROS_WORKON_SUBTREE="vm_tools/vmc"
CROS_WORKON_INCREMENTAL_BUILD=1

inherit cros-workon cros-rust cros-protobuf

DESCRIPTION="Command-line client for controlling crostini"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform2/+/HEAD/vm_tools/vmc/"

LICENSE="BSD-Google"
SLOT="0"
KEYWORDS="~*"

COMMON_DEPEND="
	chromeos-base/vboot_reference:=
	sys-apps/dbus:=
"

DEPEND="${COMMON_DEPEND}
	chromeos-base/system_api
	dev-rust/libchromeos:=
	dev-rust/system_api:=
	dev-rust/third-party-crates-src:=
"

RDEPEND="${COMMON_DEPEND}"

RESTRICT="!x86? ( !amd64? ( test ) )"

src_unpack() {
	cros-workon_src_unpack
	# The compilation happens in the vmc subdirectory.
	S+="/vm_tools/vmc"
	cros-rust_src_unpack
}

src_compile() {
	ecargo_build
	use test && ecargo_test --no-run
}

src_test() {
	ecargo_test --all
}

src_install() {
	local build_dir="$(cros-rust_get_build_dir)"
	dobin "${build_dir}/vmc"
}
