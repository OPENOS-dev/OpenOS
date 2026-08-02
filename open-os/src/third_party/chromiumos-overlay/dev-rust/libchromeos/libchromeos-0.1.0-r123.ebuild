# Copyright 2019 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_WORKON_COMMIT="2bc5cd2f0ce704a61d888f178013a15bf6fe83c8"
CROS_WORKON_TREE="4b72b2dd14e6f0281b6b850005c2c2a5babe3d4a"
CROS_RUST_SUBDIR="libchromeos-rs"

CROS_WORKON_INCREMENTAL_BUILD=1
CROS_WORKON_LOCALNAME="../platform2"
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_SUBTREE="${CROS_RUST_SUBDIR}"

inherit cros-workon cros-rust

DESCRIPTION="A Rust utility library for Chrome OS"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform2/+/HEAD/libchromeos-rs/"

LICENSE="BSD-Google"
KEYWORDS="*"
IUSE="test"

COMMON_DEPEND="
	dev-rust/third-party-crates-src:=
	>=dev-rust/poll_token_derive-0.1.1:=
	dev-rust/system_api:=
	dev-rust/vboot_reference-sys:=
	sys-apps/dbus:=
"

DEPEND="
	${COMMON_DEPEND}
	virtual/bindgen:=
"

RDEPEND="
	${COMMON_DEPEND}
	!!<=dev-rust/libchromeos-0.1.0-r2
"

BDEPEND="
	dev-rust/bindgen
"

src_compile() {
	# Make sure the build works with default features.
	ecargo_build
	# Also check that the build works with all features.
	ecargo_build --all-features
	use test && cros-rust_src_test --no-run --all-features
}

src_test() {
	cros-rust_src_test --all-features -- --test-threads=1
}
