# Copyright 2024 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI="7"

CROS_RUST_CRATE_NAME=blazesym-c
CROS_RUST_CRATE_VERSION=0.1.0-rc.2
CROS_RUST_REMOVE_DEV_DEPS=1
CROS_RUST_PREINSTALLED_REGISTRY_CRATE=1

inherit cros-rust

DESCRIPTION='C bindings for blazesym'
HOMEPAGE='https://github.com/libbpf/blazesym'

LICENSE="BSD"
SLOT="0/${PVR}"
KEYWORDS="*"

DEPEND="dev-rust/third-party-crates-src:="
RDEPEND="${DEPEND}"

src_compile() {
	ecargo_build
}

src_install() {
	doheader "include/blazesym.h"
	dolib.so "$(cros-rust_get_build_dir)/libblazesym_c.so"
	dolib.a "$(cros-rust_get_build_dir)/libblazesym_c.a"
}
