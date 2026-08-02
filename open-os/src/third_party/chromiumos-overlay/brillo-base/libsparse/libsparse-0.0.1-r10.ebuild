# Copyright 2024 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI="7"

inherit cros-toolchain-funcs

# Mirrored from: https://android.googlesource.com/platform/system/core/+archive/630e05b6af5f76bd7f063840e543186bde40ff0a.tar.gz
SRC_URI="gs://chromeos-localmirror/distfiles/aosp-platform-system-core-630e05b6af5f76bd7f063840e543186bde40ff0a.tar.gz"

DESCRIPTION="Library and cli tools for Android sparse files"
HOMEPAGE="https://android.googlesource.com/platform/system/core"

LICENSE="Apache-2.0"
SLOT="0"
KEYWORDS="*"

RDEPEND="
	sys-libs/zlib:=
"
DEPEND="${RDEPEND}"

S="${WORKDIR}/${PN}"

src_prepare() {
	default
	cp "${FILESDIR}/Makefile" "${S}" || die "Copying Makefile"
}

src_configure() {
	export GENTOO_LIBDIR=$(get_libdir)
	tc-export CC
	default
}
