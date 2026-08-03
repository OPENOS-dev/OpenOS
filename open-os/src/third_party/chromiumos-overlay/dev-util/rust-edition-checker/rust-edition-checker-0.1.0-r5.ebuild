# Copyright 2024 the ChromiumOS Authors
# Distributed under the terms of the BSD license.

EAPI=7

# NOTE: Since this project is very small and self-contained, sources are just
# located in ${FILESDIR}.
CROS_WORKON_COMMIT="d2d95e8af89939f893b1443135497c1f5572aebc"
CROS_WORKON_TREE="776139a53bc86333de8672a51ed7879e75909ac9"
CROS_WORKON_PROJECT="chromiumos/infra/build/empty-project"
CROS_WORKON_LOCALNAME="../platform/empty-project"
CROS_WORKON_OUTOFTREE_BUILD=1

# Disable edition checks of the edition checker to avoid a circular dependency.
# shellcheck disable=SC2034  # used in cros-rust.eclass.
CROS_RUST_DISABLE_EDITION_CHECKS=1

inherit cros-workon cros-rust

DESCRIPTION="Used by cros-rust.eclass to ensure 1p Rust editions are up-to-date"

LICENSE="BSD-Google"
KEYWORDS="*"
IUSE=""

DEPEND="dev-rust/third-party-crates-src:="
RDEPEND="dev-rust/third-party-crates-src:="

src_unpack() {
	cp -r "${FILESDIR}/rust-edition-checker" "${WORKDIR}" || die
	S="${WORKDIR}/rust-edition-checker"
	cros-rust_src_unpack
}

src_install() {
	dobin "$(cros-rust_get_build_dir)/rust-edition-checker"
}
