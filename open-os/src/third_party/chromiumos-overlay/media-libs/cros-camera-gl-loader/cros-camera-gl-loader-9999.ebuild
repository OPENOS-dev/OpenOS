# Copyright 2024 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_LOCALNAME="../platform2"
CROS_WORKON_SUBTREE=".gn camera/build camera/include camera/gpu/gl_loader common-mk"
CROS_WORKON_OUTOFTREE_BUILD="1"
CROS_WORKON_INCREMENTAL_BUILD="1"

PLATFORM_SUBDIR="camera/gpu/gl_loader"

inherit cros-workon platform

DESCRIPTION="ChromeOS camera GL Loader"

LICENSE="BSD-Google"
KEYWORDS="~*"

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
