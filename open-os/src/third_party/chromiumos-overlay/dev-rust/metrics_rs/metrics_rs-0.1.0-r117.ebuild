# Copyright 2023 The Chromium OS Authors. All rights reserved.
# Distributed under the terms of the GNU General Public License v2

EAPI="7"
CROS_WORKON_COMMIT="362404e596160add78f63bc42ff2081b91941af5"
CROS_WORKON_TREE="3d01071ecc84cb5a5781cd4a17dc0efe279a6169"
CROS_WORKON_INCREMENTAL_BUILD=1
CROS_WORKON_LOCALNAME="../platform2"
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_SUBTREE="metrics"
CROS_RUST_SUBDIR="metrics/rust-client"

inherit cros-workon cros-rust

DESCRIPTION="Chrome OS metrics rust binding"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform2/+/HEAD/metrics/"
LICENSE="BSD-Google"
KEYWORDS="*"

RDEPEND="chromeos-base/metrics:="
DEPEND="
	${RDEPEND}
	dev-rust/third-party-crates-src:=
"
