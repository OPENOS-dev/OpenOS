# Copyright 2024 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_WORKON_INCREMENTAL_BUILD="1"
CROS_WORKON_LOCALNAME="platform2"
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_OUTOFTREE_BUILD=1
CROS_WORKON_SUBTREE="common-mk heatmap-recorder libtouchraw .gn"

PLATFORM_SUBDIR="heatmap-recorder"

inherit cros-workon platform

DESCRIPTION="SPI Heatmap Recorder Tool"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform2/+/HEAD/heatmap-recorder"

LICENSE="BSD-Google"
KEYWORDS="~*"
IUSE="input_devices_spi_heatmap"
REQUIRED_USE="input_devices_spi_heatmap"

COMMON_DEPEND="
	chromeos-base/libtouchraw:=
	dev-cpp/abseil-cpp:=
"
RDEPEND="${COMMON_DEPEND}"
DEPEND="${COMMON_DEPEND}"
BDEPEND=""
