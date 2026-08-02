# Copyright 2024 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI="7"

CROS_WORKON_COMMIT="e67643c64a105f6f744b007eb857f381ace07e8e"
CROS_WORKON_TREE=("a0f2ebc8d4dca5697c329405208b2cd08ff35478" "518b50f8b6d01e95cbd933487ed7c6452ac4acb3")
CROS_WORKON_INCREMENTAL_BUILD=1
CROS_WORKON_LOCALNAME="platform2"
CROS_WORKON_PROJECT="chromiumos/platform2"
# We don't use CROS_WORKON_OUTOFTREE_BUILD here since project's Cargo.toml is
# using "provided by ebuild" macro which supported by cros-rust.
CROS_RUST_SUBDIR="ferrochrome/clipboard"
CROS_WORKON_SUBTREE="${CROS_RUST_SUBDIR} common-mk"

inherit cros-workon cros-rust

DESCRIPTION="Clipboard sharing server daemon running in Ferrochrome"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform2/+/HEAD/ferrochrome/clipboard/"

LICENSE="BSD-Google"
KEYWORDS="*"

RDEPEND="
	acct-group/ferrochromed
	acct-user/ferrochromed
	gui-apps/wl-clipboard
"

DEPEND="
	dev-rust/libchromeos:=
	dev-rust/third-party-crates-src:=
"

src_install() {
	dobin "$(cros-rust_get_build_dir)/clipboard_sharing_server"

	insinto /etc/init
	doins init/clipboard_sharing_server.conf
}
