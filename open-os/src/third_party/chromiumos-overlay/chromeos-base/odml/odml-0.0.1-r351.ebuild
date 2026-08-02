# Copyright 2024 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

EAPI=7

CROS_WORKON_COMMIT="362404e596160add78f63bc42ff2081b91941af5"
CROS_WORKON_TREE=("518b50f8b6d01e95cbd933487ed7c6452ac4acb3" "3d01071ecc84cb5a5781cd4a17dc0efe279a6169" "6254ee88618b3a25710d425883a96ce5da6e351d" "6b4320f76712f5e5d454864cfbddf2f25918dcfa" "43eb4f30218ee6fc055f185786d914bccd668086" "099b68f0f78008ee9e77a0664f72e2e720de8e1a" "f91b6afd5f2ae04ee9a2c19109a3a4a36f7659e6")
CROS_WORKON_INCREMENTAL_BUILD=1
CROS_WORKON_LOCALNAME="platform2"
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_OUTOFTREE_BUILD=1

# TODO(crbug.com/809389): Avoid directly including headers from other packages.
CROS_WORKON_SUBTREE="common-mk metrics ml ml_core/dlc mojo_service_manager odml .gn"

PLATFORM_SUBDIR="odml"

inherit cros-protobuf cros-workon platform user

DESCRIPTION="On-device ML service"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform2/+/HEAD/odml/"

# Apache-2.0 is for the libdmabufheap implementation we ported from aosp.
LICENSE="BSD-Google Apache-2.0"
KEYWORDS="*"
IUSE="feature_management mantis"

RDEPEND="
	chromeos-base/dlcservice-client:=
	chromeos-base/metrics:=
	chromeos-base/minijail:=
	chromeos-base/mojo_service_manager:=
	chromeos-base/session_manager-client:=
	dev-libs/flatbuffers:=
	dev-libs/protobuf:=
	sci-libs/tensorflow:=
"

DEPEND="
	${RDEPEND}
	chromeos-base/system_api:=[fuzzer?]
	dev-cpp/abseil-cpp:=
"

BDEPEND="
	chromeos-base/minijail
"

src_install() {
	platform_src_install
	local daemon_store="/etc/daemon-store/odmld"
	dodir "${daemon_store}"
	fperms 0700 "${daemon_store}"
	fowners odml:odml "${daemon_store}"
}


pkg_setup() {
	# Has to be done in pkg_setup() instead of pkg_preinst() since
	# src_install() needs the odml user and group.
	enewuser odml
	enewgroup odml
	cros-workon_pkg_setup
}
