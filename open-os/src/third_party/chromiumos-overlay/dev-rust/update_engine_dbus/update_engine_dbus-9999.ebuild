# Copyright 2022 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_WORKON_INCREMENTAL_BUILD=1
CROS_WORKON_LOCALNAME="../platform2"
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_SUBTREE="update_engine"
CROS_RUST_SUBDIR="update_engine"

inherit cros-workon cros-rust

DESCRIPTION="Rust D-Bus bindings for update_engine."
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform2/+/HEAD/update_engine/"

LICENSE="BSD-Google"
SLOT="0/${PVR}"
KEYWORDS="~*"

DEPEND="
	chromeos-base/update_engine:=
	dev-rust/third-party-crates-src:=
	dev-rust/chromeos-dbus-bindings:=
	sys-apps/dbus:=
"
# (crbug.com/1182669): build-time only deps need to be in RDEPEND so they are pulled in when
# installing binpkgs since the full source tree is required to use the crate.
RDEPEND="${DEPEND}"
