# Copyright 1999-2013 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2

EAPI="7"
EGIT_REPO_URI="https://gitlab.freedesktop.org/mesa/drm.git"
if [[ ${PV} != *9999* ]]; then
	CROS_WORKON_COMMIT="38ec7dbd4df3141441afafe5ac62dfc9df36a77e"
	CROS_WORKON_TREE="5c823cde698ae73be3f7a767dce837b2da416e64"
fi
CROS_WORKON_PROJECT="chromiumos/third_party/libdrm"
CROS_WORKON_EGIT_BRANCH="upstream/master"
CROS_WORKON_MANUAL_UPREV="1"

inherit cros-workon flag-o-matic meson

DESCRIPTION="X.Org libdrm library"
HOMEPAGE="http://dri.freedesktop.org/"
SRC_URI=""

# This package uses the MIT license inherited from Xorg but fails to provide
# any license file in its source, so we add X as a license, which lists all
# the Xorg copyright holders and allows license generation to pick them up.
LICENSE="|| ( MIT X )"
SLOT="0"
if [[ ${PV} = *9999* ]]; then
	KEYWORDS="~*"
else
	KEYWORDS="*"
fi
VIDEO_CARDS="amdgpu freedreno intel nouveau omap radeon vc4 vmware"
for card in ${VIDEO_CARDS}; do
	IUSE_VIDEO_CARDS+=" video_cards_${card}"
done

IUSE="${IUSE_VIDEO_CARDS} manpages +udev"
RESTRICT="test" # see bug #236845

RDEPEND="dev-libs/libpthread-stubs
	udev? ( virtual/udev )
	video_cards_amdgpu? ( dev-util/cunit )
	video_cards_intel? ( >=x11-libs/libpciaccess-0.10 )
	!<x11-libs/libdrm-tests-2.4.58-r3
"

DEPEND="${RDEPEND}"

src_prepare() {
	eapply "${FILESDIR}"/Add-header-for-Mediatek-DRM-userspace.patch
	eapply "${FILESDIR}"/CHROMIUM-add-resource-info-header.patch
	eapply "${FILESDIR}"/CHROMIUM-nouveau-force-synchronous-pushbuffer-submis.patch

	eapply_user
}

src_configure() {
	append-lfs-flags
	cros_optimize_package_for_speed

	# reduce from 64 to the original 16 to speed up modeprobe
	append-cppflags -DDRM_MAX_MINOR=16

	local emesonargs=(
		-Dinstall-test-programs=true
		$(meson_feature video_cards_amdgpu amdgpu)
		$(meson_feature video_cards_freedreno freedreno)
		$(meson_feature video_cards_intel intel)
		$(meson_feature video_cards_nouveau nouveau)
		$(meson_feature video_cards_omap omap)
		$(meson_feature video_cards_radeon radeon)
		$(meson_feature video_cards_vc4 vc4)
		$(meson_feature video_cards_vmware vmwgfx)
		$(meson_feature manpages man-pages)
		$(meson_use udev)
		-Dcairo-tests=disabled
	)
	meson_src_configure
}
