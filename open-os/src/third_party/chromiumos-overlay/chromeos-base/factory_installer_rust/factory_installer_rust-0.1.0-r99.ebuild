# Copyright 2023 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7
CROS_WORKON_COMMIT="589921af252f5e9234d68744e22d5cd0d63ab687"
CROS_WORKON_TREE="29dd83e7260ce6ae92e3840376c0df2119fb7386"
CROS_WORKON_PROJECT="chromiumos/platform/factory_installer"
CROS_WORKON_LOCALNAME="platform/factory_installer"
CROS_RUST_CRATE_NAME="factory_installer"
CROS_RUST_SUBDIR="rust"
CROS_RUST_TEST_DIRECT_EXEC_ONLY="yes"

inherit cros-workon cros-rust

DESCRIPTION="ChromeOS Factory installer binary"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform/factory_installer/"
SRC_URI=""
LICENSE="BSD-Google"
KEYWORDS="*"
IUSE=""

DEPEND="dev-rust/third-party-crates-src:="

src_test() {
	cros-rust_src_test --no-default-features --features="factory-installer" \
		--lib
}

src_compile() {
	cros-rust_src_compile --no-default-features --features="factory-installer" \
		--bin="factory_installer"
}

src_install() {
	dosbin "$(cros-rust_get_build_dir)/factory_installer"
}
