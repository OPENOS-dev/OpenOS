# Copyright 2023 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

EAPI=7

CROS_WORKON_COMMIT="362404e596160add78f63bc42ff2081b91941af5"
CROS_WORKON_TREE=("f91b6afd5f2ae04ee9a2c19109a3a4a36f7659e6" "4775a5be81f113aa93a6867b9e5c2576fcab81e6" "9d5a2a0cdb9fce70bfb36ab315206c7927d44227" "824ea58991cc7bca28f57df8fedafdf7dec36a29" "0e7d4d4aac5fe2e42c89c6335278db4cd635aec2" "db40287b545501ab51eb84d3e0f5e66ea697d24d" "518b50f8b6d01e95cbd933487ed7c6452ac4acb3" "3d01071ecc84cb5a5781cd4a17dc0efe279a6169" "6b4320f76712f5e5d454864cfbddf2f25918dcfa" "43eb4f30218ee6fc055f185786d914bccd668086")
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_LOCALNAME="platform2"
CROS_WORKON_SUBTREE=".gn camera/build camera/common camera/include camera/mojo camera/diagnostics common-mk metrics ml_core/dlc mojo_service_manager"
CROS_WORKON_OUTOFTREE_BUILD="1"
CROS_WORKON_INCREMENTAL_BUILD="1"

PLATFORM_SUBDIR="camera/diagnostics"

inherit cros-camera cros-protobuf cros-workon platform

DESCRIPTION="ChromeOS camera diagnostics service."

LICENSE="BSD-Google"
KEYWORDS="*"
IUSE="dlc camera_diagnostics"
# This package has no unittests.
RESTRICT="test"
# Make sure we enabled the package correctly.
REQUIRED_USE="camera_diagnostics"

BDEPEND="
	chromeos-base/minijail
	virtual/pkgconfig
"

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
