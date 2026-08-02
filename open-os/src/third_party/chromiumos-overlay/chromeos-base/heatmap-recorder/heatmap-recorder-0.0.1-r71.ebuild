# Copyright 2024 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_WORKON_COMMIT="e67643c64a105f6f744b007eb857f381ace07e8e"
CROS_WORKON_TREE=("518b50f8b6d01e95cbd933487ed7c6452ac4acb3" "e6a1dd439463b3f6e5156c9e45ec0f161b258d98" "bba4bef6c0743c6bedd60561a468afd36933b086" "f91b6afd5f2ae04ee9a2c19109a3a4a36f7659e6")
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
KEYWORDS="*"
IUSE="input_devices_spi_heatmap"
REQUIRED_USE="input_devices_spi_heatmap"

COMMON_DEPEND="
	chromeos-base/libtouchraw:=
	dev-cpp/abseil-cpp:=
"
RDEPEND="${COMMON_DEPEND}"
DEPEND="${COMMON_DEPEND}"
BDEPEND=""
