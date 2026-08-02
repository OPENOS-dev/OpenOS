# Copyright 2023 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI="7"

CROS_RUST_PREINSTALLED_REGISTRY_CRATE=1

inherit cros-rust

DESCRIPTION="gRPC compiler for grpcio"
HOMEPAGE="https://github.com/tikv/grpc-rs/compiler"

LICENSE="Apache-2.0"
SLOT="0/${PVR}"
KEYWORDS="*"
IUSE="cros_host"

BDEPEND="!cros_host? ( =${CATEGORY}/${PF} )"
DEPEND="dev-rust/third-party-crates-src:="

RDEPEND="${DEPEND}"

src_compile() {
	use cros_host && ecargo_build
}

src_install() {
	use cros_host && dobin "$(cros-rust_get_build_dir)/grpc_rust_plugin"
}
