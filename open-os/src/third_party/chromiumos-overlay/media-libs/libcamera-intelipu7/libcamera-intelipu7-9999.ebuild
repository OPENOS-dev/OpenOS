# Copyright 2024 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_WORKON_PROJECT="chromiumos/third_party/libcamera"
CROS_WORKON_LOCALNAME="libcamera/intelipu7"
CROS_WORKON_EGIT_BRANCH="intelipu7"

# TODO(chenghaoyang): Update to intelipu7 when pipeline handler is added.
LIBCAMERA_PIPELINES="vimc"
# LIBCAMERA_IPA="intelipu7"

# LIBCAMERA_DEPEND=""

inherit cros-camera cros-workon libcamera

DESCRIPTION="Camera support library for Linux on intelipu7"

KEYWORDS="~*"
