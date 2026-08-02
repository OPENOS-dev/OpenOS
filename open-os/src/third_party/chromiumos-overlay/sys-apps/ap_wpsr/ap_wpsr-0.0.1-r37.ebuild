# Copyright 2023 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2
# $Header:

EAPI=7

# ap_wpsr is a sub-project within the main flashrom repository. Its Meson
# build files require access to the parent directory (e.g., for ../../ headers),
# so we must check out the entire flashrom project. EMESON_SOURCE is then
# used in src_configure to direct the build to the correct subdirectory.
CROS_WORKON_COMMIT="ba341e63b66cb0d7779b138aeff1644ec0fbef14"
CROS_WORKON_TREE="000c575b291ae659d2764455e9e03e29dcaf9639"
CROS_WORKON_PROJECT="chromiumos/third_party/flashrom"
CROS_WORKON_LOCALNAME="flashrom"

inherit cros-workon cros-toolchain-funcs meson cros-sanitizers

DESCRIPTION="Utility for generating AP WP masks and values"
HOMEPAGE=""
SRC_URI=""

LICENSE="GPL-2"
SLOT="0/0"
KEYWORDS="*"
IUSE=""

RDEPEND=""
DEPEND=""

src_configure() {
	# Set the ap_wpsr source directory for meson, as defined in meson.eclass.
	EMESON_SOURCE="${S}/util/ap_wpsr"
	sanitizers-setup-env
	meson_src_configure
}

src_install() {
	meson_src_install
}
