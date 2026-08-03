# Copyright 2024 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

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
KEYWORDS="~*"
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
