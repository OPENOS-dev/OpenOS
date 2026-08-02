# Copyright 2024 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

EAPI=7

CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_LOCALNAME="../platform2"
CROS_WORKON_SUBTREE=".gn camera/build camera/common camera/include camera/mojo camera/diagnostics camera/diagnostics/tests common-mk metrics ml_core/dlc mojo_service_manager"
CROS_WORKON_OUTOFTREE_BUILD="1"
CROS_WORKON_INCREMENTAL_BUILD="1"

PLATFORM_SUBDIR="camera/diagnostics/tests"

inherit cros-camera cros-protobuf cros-workon platform

DESCRIPTION="ChromeOS camera diagnostics service tests."

LICENSE="BSD-Google"
KEYWORDS="~*"
IUSE="dlc camera_diagnostics"
# This package has no unittests.
RESTRICT="test"
# Make sure we enabled the package correctly.
REQUIRED_USE="camera_diagnostics"

BDEPEND="virtual/pkgconfig"

RDEPEND="
	chromeos-base/cros-camera-libs:=
	chromeos-base/dlcservice-client:=
	chromeos-base/metrics:=
	chromeos-base/mojo_service_manager:=
	chromeos-base/system_api:=
	dlc? (
		media-libs/cros-camera-blur-detector-dlc:=
	)"

DEPEND="${RDEPEND}"
