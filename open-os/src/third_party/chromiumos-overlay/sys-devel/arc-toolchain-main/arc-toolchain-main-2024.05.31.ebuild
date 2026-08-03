# Copyright 1999-2024 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header: $

EAPI="7"

DESCRIPTION="Ebuild for Android toolchain (compilers, linker, libraries, headers)."

# The source tarball contains files collected from the sources below.
#
#   # from ab/11914485
#   bertha_arm64-target_files-11914485.zip
#   bertha_x86_64-target_files-11914485.zip
#
#   platform/bionic revision: 77167715271cef05550d23f2b6491b28e818ae6b
#   platform/system/apex revision: baec90dad1615187f0e06fa81e6e8cb42fa1d6c2
#   platform/external/e2fsprogs revision: 8ed9be20467278848ef06a17915f697844c07cc2
#   platform/external/erofs-utils revision: 45ee690acb651d56e11f254a5632618ab2bc0692
#
SRC_URI="http://commondatastorage.googleapis.com/chromeos-localmirror/distfiles/${P}.tar.gz"

LICENSE="GPL-3 LGPL-3 GPL-3 libgcc libstdc++ gcc-runtime-library-exception-3.1 FDL-1.2 UoI-NCSA"
SLOT="0"
KEYWORDS="-* amd64"
IUSE=""

BDEPEND="
	app-misc/pax-utils
"

S="${WORKDIR}"
INSTALL_DIR="/opt/android-main"

# These prebuilts are already properly stripped.
RESTRICT="strip"
QA_PREBUILT="*"

src_install() {
	dodir "${INSTALL_DIR}"
	# Remove unused libraries which depend on ncurses:5
	# shellcheck disable=SC2046
	rm -f $(scanelf -qRN libtinfo.so.5 .|awk '{print $2}')
	cp -pPR ./* "${D}/${INSTALL_DIR}/" || die
}
