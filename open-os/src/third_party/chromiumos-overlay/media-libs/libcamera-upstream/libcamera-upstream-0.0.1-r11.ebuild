# Copyright 2023 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_WORKON_COMMIT="fc268340790c34e9a36bf0bce73a4f895325256f"
CROS_WORKON_TREE="3a581f873c009de4a15f95c087ef1207315eaae7"
CROS_WORKON_PROJECT="chromiumos/third_party/libcamera"
CROS_WORKON_LOCALNAME="libcamera/upstream"

LIBCAMERA_PIPELINES="auto"

LIBCAMERA_DEPEND=""

inherit cros-camera cros-workon libcamera

DESCRIPTION="Camera support library for Linux"

KEYWORDS="*"
