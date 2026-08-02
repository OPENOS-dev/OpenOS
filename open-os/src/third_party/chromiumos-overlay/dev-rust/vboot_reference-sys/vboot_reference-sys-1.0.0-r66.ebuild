# Copyright 2022 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_WORKON_COMMIT="399940cfb4a53f0d77f89c1bc22e3016134a6cb7"
CROS_WORKON_TREE=("866dcf88013fd87a0d96736f15bf359649ed1ff9" "12f0d35efcf49276e899ba2cca2d5599f052ac02" "048fd199c04744272c351e5949f827874f66476a" "64d8ead44395e8ec58fb83826f014e178f1f0558")
inherit cros-constants

CROS_RUST_SUBDIR="rust/vboot_reference-sys"

CROS_WORKON_PROJECT="chromiumos/platform/vboot_reference"
CROS_WORKON_LOCALNAME="../platform/vboot_reference"
CROS_WORKON_SUBTREE="host/include firmware/include firmware/2lib/include rust/vboot_reference-sys"

inherit cros-workon cros-rust

DESCRIPTION="Chrome OS verified boot tools Rust bindings"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform/vboot_reference/+/HEAD/rust"

LICENSE="BSD-Google"
KEYWORDS="*"

# ebuilds that install executables and depend on vboot_reference-sys need to
# RDEPEND on chromeos-base/vboot_reference
DEPEND="
	dev-rust/third-party-crates-src:=
	chromeos-base/vboot_reference:=
	virtual/bindgen:=
"
# (crbug.com/1182669): build-time only deps need to be in RDEPEND so they are pulled in when
# installing binpkgs since the full source tree is required to use the crate.
RDEPEND="${DEPEND}"

BDEPEND="
	dev-rust/bindgen
"
