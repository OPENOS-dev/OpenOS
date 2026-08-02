# Copyright 1999-2024 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7
CROS_RUST_CRATE_NAME="thin-provisioning-tools"
CROS_RUST_SUBDIR="rust"

inherit cros-rust

DESCRIPTION="A suite of tools for thin provisioning on Linux"
HOMEPAGE="https://github.com/jthornber/thin-provisioning-tools"

SRC_URI="https://github.com/jthornber/${PN}/archive/v${PV}.tar.gz -> ${P}.tar.gz"
KEYWORDS="*"

LICENSE="GPL-3"
SLOT="0"

DOCS=(
	CHANGES
	COPYING
	README.md
	doc/TODO.md
	doc/thinp-version-2/notes.md
)

DEPEND="dev-rust/third-party-crates-src:="

PATCHES=(
	"${FILESDIR}/${PN}-1.0.6-build-with-cargo.patch"
	"${FILESDIR}/${PN}-1.0.13-remove-io_uring-dev-features.patch"
)

src_compile() {
	ecargo_build "$@"
}
src_install() {
	# CARGO_TARGET_DIR and CHOST defined in an eclass
	# shellcheck disable=SC2154
	local release_dir="${CARGO_TARGET_DIR}/${CHOST}/release"
	emake \
		DESTDIR="${D}" \
		DATADIR="${ED}/usr/share" \
		PDATA_TOOLS="${release_dir}/pdata_tools" \
		install

	einstalldocs
}
