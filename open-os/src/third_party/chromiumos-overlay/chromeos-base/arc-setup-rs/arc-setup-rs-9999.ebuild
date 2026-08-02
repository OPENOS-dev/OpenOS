# Copyright 2024 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI="7"
CROS_WORKON_INCREMENTAL_BUILD=1
CROS_WORKON_LOCALNAME="platform2"
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_SUBTREE="arc/setup"
CROS_RUST_SUBDIR="arc/setup/rust"

inherit cros-workon cros-rust

DESCRIPTION="arc-setup Rust components"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform2/+/HEAD/arc/setup/rust"
LICENSE="BSD-Google"
KEYWORDS="~*"

DEPEND="
	dev-rust/third-party-crates-src:=
"

RDEPEND="
	dev-rust/third-party-crates-src:=
"

BDEPEND="
	dev-rust/third-party-crates-src:=
"

pkg_setup() {
	cros-rust_pkg_setup
}

src_compile() {
	# Remove other versions of arc-setup-rs
	rm -rf "$(cros-rust_get_build_dir)/build/arc-setup-rs-*" ||
		die "failed to remove old arc-setup-rs packages"

	ecargo_build -v -p arc-setup-rs || die "arc-setup-rs cargo build failed"
}

src_install() {
	local build_dir="$(cros-rust_get_build_dir)"

	# Install arc-setup-rs header and library.
	insinto /usr/include/arc-setup
	doins "${build_dir}"/build/arc-setup-rs-*/out/arc-setup-rs.h
	dolib.so "${build_dir}/deps/libarc_setup_rs.so"
}
