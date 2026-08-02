# Copyright 2024 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI="7"

CROS_RUST_REMOVE_DEV_DEPS=1

CROS_WORKON_LOCALNAME="../aosp/build"
CROS_WORKON_PROJECT="platform/build"
CROS_RUST_CRATE_NAME="aconfig"
CROS_WORKON_INCREMENTAL_BUILD=0
CROS_RUST_SUBDIR="tools/aconfig/aconfig"

inherit cros-workon cros-rust

DESCRIPTION='aconfig is a build time tool to manage build time configurations, such as feature flags.'
HOMEPAGE='https://android.googlesource.com/platform/build/+/refs/heads/main/tools/aconfig/'

LICENSE="Apache-2.0"
KEYWORDS="~*"

DEPEND="dev-rust/third-party-crates-src:="
RDEPEND="${DEPEND}"

src_compile() {
	# Check if cxxflags has -fno-exceptions and set -DRUST_CXX_NO_EXCEPTIONS
	# This is required to build the cxx rust dependency
	if is-flagq -fno-exceptions; then
		append-cxxflags -DRUST_CXX_NO_EXCEPTIONS
	fi

	ecargo_build
}

src_install() {
	dobin "$(cros-rust_get_build_dir)/aconfig"
}
