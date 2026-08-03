# Copyright 1999-2024 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

inherit meson cros-sanitizers

MY_PN=mesa-demos
MY_P=${MY_PN}-${PV}

DESCRIPTION="eglinfo from Mesa demos"
HOMEPAGE="https://www.mesa3d.org/ https://mesa.freedesktop.org/ https://gitlab.freedesktop.org/mesa/demos"
SRC_URI="https://mesa.freedesktop.org/archive/demos/${MY_P}.tar.xz
	https://mesa.freedesktop.org/archive/demos/${PV}/${MY_P}.tar.xz"
KEYWORDS="*"

LICENSE="LGPL-2"
SLOT="0"

RDEPEND="
	virtual/opengles
"
DEPEND="${RDEPEND}
	x11-drivers/opengles-headers
"
BDEPEND="
	virtual/pkgconfig
"

S="${WORKDIR}/${MY_P}"

PATCHES=(
	"${FILESDIR}"/${PV}-Disable-things-we-don-t-want.patch
	"${FILESDIR}"/FROMLIST-BACKPORT-eglinfo-Handle-EGL_EXT_platform_base-without-a-platf.patch
)

src_prepare() {
	default
	sed -i -e "s/dependency('gl')/disabler()/" meson.build || die
}

src_configure() {
	sanitizers-setup-env

	local emesonargs=(
		-Dlibdrm=disabled
		-Degl=enabled
		-Dgles1=disabled
		-Dgles2=disabled
		-Dglut=disabled
		-Dosmesa=disabled
		-Dwayland=disabled
		-Dvulkan=disabled
		-Dx11=disabled
	)
	meson_src_configure
}
