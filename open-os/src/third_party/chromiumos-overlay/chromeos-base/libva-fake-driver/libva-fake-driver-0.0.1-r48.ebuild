# Copyright 2020 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI="7"

CROS_WORKON_COMMIT=("e67643c64a105f6f744b007eb857f381ace07e8e" "04f902b9ba9f8083b19d22e7b55591bbcaae31c0")
CROS_WORKON_TREE=("518b50f8b6d01e95cbd933487ed7c6452ac4acb3" "f91b6afd5f2ae04ee9a2c19109a3a4a36f7659e6" "2f4046826335833d67b5b6afc3837e66776e3874")
CROS_WORKON_INCREMENTAL_BUILD="1"
CROS_WORKON_LOCALNAME=(
	"platform2"
	"platform/libva-fake-driver"
)
CROS_WORKON_PROJECT=(
	"chromiumos/platform2"
	"chromiumos/platform/libva-fake-driver"
)
CROS_WORKON_SUBTREE=(
	"common-mk .gn"
	""
)
CROS_WORKON_DESTDIR=(
	"${S}/platform2"
	"${S}/platform2/libva-fake-driver"
)

PLATFORM_SUBDIR="libva-fake-driver"

inherit cros-workon platform

DESCRIPTION="ChromeOS fake LibVA driver; intended as a backend replacement for VMs and other test fixtures"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform/libva-fake-driver/"

LICENSE="BSD-Google"
KEYWORDS="*"

RDEPEND="
	>=x11-libs/libva-2.6.0:=
	media-libs/minigbm:=
	media-libs/libyuv:=
	media-libs/libvpx:=
	media-libs/dav1d:=
	media-libs/openh264:=
"
DEPEND="${RDEPEND}"
BDEPEND="virtual/pkgconfig"

src_install() {
	into "/usr/$(get_libdir)/va/drivers"
	dolib.so "${OUT}/lib/libfake_drv_video.so" "${OUT}/lib/libfake_gbm.so"
	dosym "./$(get_libdir)/libfake_drv_video.so" \
		"/usr/$(get_libdir)/va/drivers/libfake_drv_video.so"
}
