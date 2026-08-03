# Copyright 2024 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_WORKON_COMMIT="e67643c64a105f6f744b007eb857f381ace07e8e"
CROS_WORKON_TREE=("518b50f8b6d01e95cbd933487ed7c6452ac4acb3" "75bfe639161c914b6a74af164a3a5c65bc8950d0" "f91b6afd5f2ae04ee9a2c19109a3a4a36f7659e6")
CROS_WORKON_LOCALNAME="platform2"
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_OUTOFTREE_BUILD=1
CROS_WORKON_INCREMENTAL_BUILD=1
CROS_WORKON_SUBTREE="common-mk patchmaker .gn"

PLATFORM_SUBDIR="patchmaker"

inherit cros-workon platform cros-protobuf

DESCRIPTION="Chrome OS utility to manage binary patches"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform2/+/HEAD/patchmaker"

LICENSE="BSD-Google"
KEYWORDS="*"
IUSE=""

# The bsdiff package installs static libs only, so we have to depend on its
# libs directly so we're rebuilt when their ABI changes too, even if we don't
# use the libs ourselves.
BSDIFF_DEPEND="
	app-arch/brotli:=
	app-arch/bzip2:=
	dev-util/bsdiff:=
	dev-libs/libdivsufsort:=
"

COMMON_DEPEND="
	${BSDIFF_DEPEND}
	app-arch/zstd:=
"

DEPEND="
	${COMMON_DEPEND}
"

RDEPEND="
	${COMMON_DEPEND}
"

src_install() {
	platform_src_install
	dobin "${OUT}"/patchmaker
}
