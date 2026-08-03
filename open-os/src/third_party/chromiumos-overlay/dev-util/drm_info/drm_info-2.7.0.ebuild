# Copyright 2024 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

inherit meson

if [[ ${PV} == "9999" ]] ; then
	EGIT_REPO_URI="https://gitlab.freedesktop.org/emersion/drm_info.git"
	inherit git-r3
	SRC_URI=""
else
	SRC_URI="https://gitlab.freedesktop.org/emersion/drm_info/-/archive/v${PV}/${PN}-v${PV}.tar.bz2"
	KEYWORDS="*"
	S="${WORKDIR}/${PN}-v${PV}"
fi

DESCRIPTION="Small utility to dump info about DRM devices"
HOMEPAGE="https://gitlab.freedesktop.org/emersion/drm_info"
LICENSE="MIT"
SLOT="0"

DEPEND=">=dev-libs/json-c-0.14
	>=x11-libs/libdrm-2.4.122
	x86? ( sys-apps/pciutils )
	amd64? ( sys-apps/pciutils )
"
RDEPEND="${DEPEND}"

src_configure() {
	if use x86 || use amd64; then
		emesonargs+=( -Dlibpci=enabled )
	fi

	meson_src_configure
}
