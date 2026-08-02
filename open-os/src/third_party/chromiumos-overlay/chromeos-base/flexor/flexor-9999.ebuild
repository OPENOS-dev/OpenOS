# Copyright 2023 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

EAPI=7

CROS_WORKON_INCREMENTAL_BUILD=1
CROS_WORKON_LOCALNAME="platform2"
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_SUBTREE="flexor"

inherit cros-workon cros-rust

DESCRIPTION="Contains the main logic for the Flexor experimental Flex installer"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform2/+/HEAD/flexor/"

LICENSE="BSD-Google"
KEYWORDS="~*"
IUSE="test flexor_debug"

DEPEND="
	dev-rust/third-party-crates-src:=
	dev-rust/libchromeos:=
	app-arch/xz-utils
	chromeos-base/vboot_reference
"

RDEPEND=""

src_prepare() {
	# We don't want xtask dependencies to be evaluated,
	# that's just a dev tool and can use stuff not in rust_crates.
	sed -i 's:"xtask"::' "${S}/Cargo.toml" || die

	cros-rust_src_prepare
}

src_install() {
	into "/build/initramfs"
	dosbin "$(cros-rust_get_build_dir)/flexor"

	# Upstart configs
	insinto "/build/initramfs/"
	doins -r ramfs/*
	if use flexor_debug; then
		doins -r ramfs_debug/*
	fi
}
