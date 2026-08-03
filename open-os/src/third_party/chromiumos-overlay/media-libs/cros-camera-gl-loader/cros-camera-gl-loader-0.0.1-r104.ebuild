# Copyright 2024 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_WORKON_COMMIT="e67643c64a105f6f744b007eb857f381ace07e8e"
CROS_WORKON_TREE=("f91b6afd5f2ae04ee9a2c19109a3a4a36f7659e6" "4775a5be81f113aa93a6867b9e5c2576fcab81e6" "824ea58991cc7bca28f57df8fedafdf7dec36a29" "4166058e843869bcb125c9a1b006cd2275081984" "518b50f8b6d01e95cbd933487ed7c6452ac4acb3")
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_LOCALNAME="../platform2"
CROS_WORKON_SUBTREE=".gn camera/build camera/include camera/gpu/gl_loader common-mk"
CROS_WORKON_OUTOFTREE_BUILD="1"
CROS_WORKON_INCREMENTAL_BUILD="1"

PLATFORM_SUBDIR="camera/gpu/gl_loader"

inherit cros-workon platform

DESCRIPTION="ChromeOS camera GL Loader"

LICENSE="BSD-Google"
KEYWORDS="*"

BDEPEND="virtual/pkgconfig"

RDEPEND="
	virtual/opengles:=
"

DEPEND="${RDEPEND}
	x11-drivers/opengles-headers:=
"

src_configure() {
	cros_optimize_package_for_speed
	platform_src_configure
}

src_install() {
	platform_src_install
	dodir "/usr/$(get_libdir)/camera_gl_loader"
	dosym "/usr/$(get_libdir)/libcamera_egl_loader.so" "/usr/$(get_libdir)/camera_gl_loader/libEGL.so.1"
	dosym "/usr/$(get_libdir)/libcamera_gles_loader.so" "/usr/$(get_libdir)/camera_gl_loader/libGLESv2.so.2"
}
