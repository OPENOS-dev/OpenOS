# Copyright 2024 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI="7"

CROS_WORKON_COMMIT="01f26323228a2eed7106f7cb07b31d68c48f155f"
CROS_WORKON_TREE="2d269b59649d73da09498f067ae99d7afcec14e4"
CROS_WORKON_INCREMENTAL_BUILD=1
CROS_WORKON_LOCALNAME="platform2"
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_SUBTREE="soul/reaper"
CROS_RUST_SUBDIR="soul/reaper"

inherit cros-workon cros-rust

DESCRIPTION="A syslog daemon"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform2/+/HEAD/soul/reaper/"

LICENSE="BSD-Google"
SLOT="0/${PVR}"
KEYWORDS="*"

IUSE="test"

DEPEND="
	dev-rust/third-party-crates-src:=
"
# (crbug.com/1182669): build-time only deps need to be in RDEPEND so they are
# pulled in when installing binpkgs since the full source tree is required to
# use the crate.
RDEPEND="${DEPEND}"

src_compile() {
	local features=(
		chromeos
	)

	ecargo_build -v \
		--features="${features[*]}" ||
		die "cargo build failed"
}

src_install() {
	dobin "$(cros-rust_get_build_dir)/reaper"
}
