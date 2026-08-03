# Copyright 1999-2011 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header: /var/cvsroot/gentoo-x86/sys-apps/flashrom/flashrom-0.9.4.ebuild,v 1.5 2011/09/20 16:03:21 nativemad Exp $

#########
# WARNING!
# This is for use only for legacy users of the ChromeOS flashrom
# cros_ec driver.
##########

EAPI=7

CROS_WORKON_COMMIT="e1bf80f4c06f7830b236daaf4602227dccce90c2"
CROS_WORKON_TREE="71b74128005c28e379cb587943868d180578b74e"
CROS_WORKON_PROJECT="chromiumos/third_party/flashrom"
CROS_WORKON_LOCALNAME="flashrom-legacy"

inherit cros-workon cros-toolchain-funcs meson cros-sanitizers

DESCRIPTION="Legacy backend drv for the fingerprint stack (bio_fw_updater)."
HOMEPAGE="https://flashrom.org/"
SRC_URI=""

LICENSE="GPL-2"
KEYWORDS="*"
IUSE=""

src_configure() {
	local emesonargs=(
		--prefix=/opt
		-Ddefault_programmer_name=cros_ec
		-Dprogrammer="dummy"
		-Ddocumentation=disabled
		-Dman-pages=disabled
		-Dtests=disabled
		-Dich_descriptors_tool=disabled
	)
	sanitizers-setup-env
	meson_src_configure
}

src_install() {
	# The following is adapted from meson.eclass's meson_install function.
	local mesoninstallargs=(
		-C "${BUILD_DIR}"
		# Change destdir from "${D}" to ${T} to be able to select and rename
		# only the sbin/flashrom binary.
		--destdir "${T}"
		--no-rebuild
		"$@"
	)

	set -- meson install "${mesoninstallargs[@]}"
	echo "$@" >&2
	"$@" || die "install failed"

	# Only install a renamed version of the flashrom binary.
	into "/opt"
	newsbin "${T}/opt/sbin/flashrom" "crosec-legacy-drv"
}
