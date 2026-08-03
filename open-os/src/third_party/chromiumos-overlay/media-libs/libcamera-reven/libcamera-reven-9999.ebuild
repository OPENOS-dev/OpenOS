# Copyright 2025 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_WORKON_PROJECT="chromiumos/third_party/libcamera"
CROS_WORKON_LOCALNAME="libcamera/reven"
CROS_WORKON_EGIT_BRANCH="reven"

LIBCAMERA_PIPELINES="ipu3"
LIBCAMERA_DEPEND=""

inherit cros-camera cros-workon libcamera

DESCRIPTION="Camera support library for Linux on reven"

KEYWORDS="~*"
