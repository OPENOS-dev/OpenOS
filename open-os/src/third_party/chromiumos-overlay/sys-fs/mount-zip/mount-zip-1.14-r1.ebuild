# Copyright 2021 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

inherit cros-sanitizers cros-toolchain-funcs

DESCRIPTION="FUSE file system for ZIP archives"
HOMEPAGE="https://github.com/google/mount-zip"
SRC_URI="https://github.com/google/${PN}/archive/v${PV}.tar.gz -> ${P}.tar.gz"

LICENSE="GPL-3+"
SLOT="0"
KEYWORDS="*"

RDEPEND="
	>=chromeos-base/chrome-icu-89:=
	dev-libs/libzip:=
	sys-fs/fuse:0=
"

DEPEND="
	${RDEPEND}
	dev-libs/boost
"

BDEPEND="
	virtual/pkgconfig
"

DOCS=( changelog README.md )

PATCHES=(
	"${FILESDIR}/${PN}-1.14-has-path.patch"
)

src_configure() {
	sanitizers-setup-env
	# ChromeOS defaults to -fno-sanitize=vptr to accommodate some packages
	# which don't build with -fsanitize=vptr. However, we actually want
	# -fsanitize=vptr because (1) more checks are good and (2) we link in
	# some objects which have -fsanitize=vptr, which *requires*
	# -fsanizite=vptr when linking.
	replace-flags -fno-sanitize=vptr -fsanitize=vptr
	cros_enable_cxx_exceptions
	tc-export AR CC CXX PKG_CONFIG
	export FUSE_MAJOR_VERSION=2
	export CHROME_ICU=1
	export PREFIX=/usr
	default
}

src_install() {
	default
	doman mount-zip.1
}

src_test() {
	addwrite /dev/fuse
	default
}
