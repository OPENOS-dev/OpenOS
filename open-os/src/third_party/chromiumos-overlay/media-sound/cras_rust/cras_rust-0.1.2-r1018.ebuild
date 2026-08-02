# Copyright 2019 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI="7"

CROS_WORKON_COMMIT="d1f7f55fb1ea2eaf50873d7d6000b3939400a667"
CROS_WORKON_TREE="da778b06b6f407cf94c9a02476ef0d877b82bc24"
CROS_RUST_SUBDIR="."

# b/333518707
# shellcheck disable=SC2034  # used in cros-rust.eclass.
CROS_RUST_FORCE_STATIC_LINK=1

CROS_WORKON_LOCALNAME="adhd"
CROS_WORKON_PROJECT="chromiumos/third_party/adhd"
# We don't use CROS_WORKON_OUTOFTREE_BUILD here since cras/src/server/rust is
# using the `provided by ebuild` macro from the cros-rust eclass

inherit cros-workon cros-rust

CROS_WORKON_INCREMENTAL_BUILD=1

DESCRIPTION="Rust code which is used within cras"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/third_party/adhd/+/HEAD"

LICENSE="BSD-Google"
KEYWORDS="*"
IUSE="dlc test"

DEPEND="
	dev-libs/openssl:=
	dev-rust/featured:=
	dev-rust/libchromeos:=
	dev-rust/minijail:=
	dev-rust/system_api:=
	dev-rust/third-party-crates-src:=
	media-libs/speex:=
	sys-apps/dbus:=
"
# (crbug.com/1182669): build-time only deps need to be in RDEPEND so they are pulled in when
# installing binpkgs since the full source tree is required to use the crate.
RDEPEND="
	${DEPEND}
	!media-sound/audio_processor
	!<media-sound/adhd-0.0.7
	media-sound/sof-tools
"

src_compile() {
	local features=(
		$(usex dlc dlc "")
		"chromiumos"
	)
	cros-rust_src_compile --features="${features[*]}" --workspace
}

src_test() {
	local features=(
		$(usex dlc dlc "")
	)
	cros-rust_src_test --features="${features[*]}" --workspace
}

src_install() {
	dolib.a "$(cros-rust_get_build_dir)/libcras_rust.a"
	dobin "$(cros-rust_get_build_dir)/audio-worker"
	dobin "$(cros-rust_get_build_dir)/cras_server_tool"
	dobin "$(cros-rust_get_build_dir)/sof_helper"

	# Install to /usr/local so they are stripped out of the release image.
	into /usr/local
	dobin "$(cros-rust_get_build_dir)/offline-pipeline"
	dobin "$(cros-rust_get_build_dir)/rock"
}
