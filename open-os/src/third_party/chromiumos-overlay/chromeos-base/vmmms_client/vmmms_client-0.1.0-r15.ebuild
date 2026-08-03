# Copyright 2019 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_WORKON_COMMIT="3033506f66f1d2fd4126dce343610ac2d124dca5"
CROS_WORKON_TREE="80600e71abf2a6b2b7926fb5873b9bd5dc67ac72"
CROS_RUST_SUBDIR="vm_tools/vmmms_client"

CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_LOCALNAME="platform2"
CROS_WORKON_SUBTREE="${CROS_RUST_SUBDIR}"

inherit cros-workon cros-rust cros-protobuf user

DESCRIPTION="vmmms_client"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform2/+/HEAD/vm_tools/vmmms_client"

LICENSE="BSD-Google"
SLOT="0/0"
KEYWORDS="*"
IUSE="kvm_guest"

DEPEND="
	dev-rust/third-party-crates-src:=
	dev-rust/system_api:=
	dev-rust/libchromeos:=
"

src_compile() {
	ecargo_build
	use test && ecargo_test --no-run --workspace
}

src_test() {
	cros-rust_src_test --workspace
}

src_install() {
	local build_dir="$(cros-rust_get_build_dir)"

	dobin "${build_dir}/vmmms_client"
}

pkg_preinst() {
	cros-rust_pkg_preinst
}
