# Copyright 2020 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_WORKON_LOCALNAME="adhd"
CROS_WORKON_PROJECT="chromiumos/third_party/adhd"
# We don't use CROS_WORKON_OUTOFTREE_BUILD here since sound_card_init/Cargo.toml
# is using "provided by ebuild" macro which supported by cros-rust
CROS_WORKON_INCREMENTAL_BUILD=1
CROS_WORKON_SUBTREE="sound_card_init"

inherit cros-workon cros-rust udev user

DESCRIPTION="Sound Card Initializer"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/third_party/adhd/+/HEAD/sound_card_init"

LICENSE="BSD-Google"
KEYWORDS="~*"

RDEPEND="
	acct-user/sound_card_init
	acct-group/sound_card_init
"

DEPEND="
	>=chromeos-base/metrics-0.0.1-r3763:=
	dev-rust/third-party-crates-src:=
	dev-rust/libchromeos:=
	dev-rust/metrics_rs:=
	>=media-libs/alsa-lib-1.2.8-r2:=
	media-sound/audio_streams:=
	media-sound/cros_alsa:=
	media-sound/cras-client:=
	media-sound/sof_sys:=
"

BDEPEND="
	chromeos-base/minijail
"

src_prepare() {
	cros-rust_src_prepare
	cros-rust-patch-cargo-toml "${S}/amp/Cargo.toml"
	cros-rust-patch-cargo-toml "${S}/dsm/Cargo.toml"
}

src_install() {
	dobin "$(cros-rust_get_build_dir)/sound_card_init"

	# Add upstart job for sound_card_init.
	insinto /etc/init
	doins sound_card_init.conf

	# Install seccomp policy file.
	insinto /usr/share/policy
	newins "seccomp/sound_card_init-seccomp-${ARCH}.policy" sound_card_init-seccomp.policy

	udev_dorules 99-sound_card_init.rules

}
