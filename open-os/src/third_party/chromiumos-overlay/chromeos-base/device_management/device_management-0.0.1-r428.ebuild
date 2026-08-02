# Copyright 2023 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_WORKON_COMMIT="362404e596160add78f63bc42ff2081b91941af5"
CROS_WORKON_TREE=("518b50f8b6d01e95cbd933487ed7c6452ac4acb3" "41a12df837a393cabb15aa3e428efaf95b170a6f" "6a9a79d4054ab44a049d1756dc7e3cce63bc2cf8" "b8c09b0737d26e92e8c1543f785a92a112de09cc" "00e60203a732c85c12f77c0e13be1a50a6819c91" "3d01071ecc84cb5a5781cd4a17dc0efe279a6169" "f91b6afd5f2ae04ee9a2c19109a3a4a36f7659e6")
CROS_WORKON_INCREMENTAL_BUILD=1
CROS_WORKON_LOCALNAME="platform2"
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_OUTOFTREE_BUILD=1
CROS_WORKON_SUBTREE="common-mk device_management libhwsec libhwsec-foundation libstorage metrics .gn"

PLATFORM_SUBDIR="device_management"

inherit cros-workon platform cros-protobuf user

DESCRIPTION="Device Management service for ChromiumOS"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform2/+/HEAD/device_management/"
SRC_URI=""

LICENSE="BSD-Google"
KEYWORDS="*"
IUSE="test tpm tpm_dynamic tpm_insecure_fallback tpm2"

RDEPEND="
	chromeos-base/libhwsec:=[test?]
	chromeos-base/libhwsec-foundation:=
	chromeos-base/device_management-client:=
	chromeos-base/libstorage:=
	chromeos-base/metrics:=
	chromeos-base/minijail:=
	dev-libs/openssl:=
"

DEPEND="${RDEPEND}
	chromeos-base/system_api:=
"

BDEPEND="
	chromeos-base/chromeos-dbus-bindings
	chromeos-base/minijail
"

pkg_preinst() {
	enewuser "device_management"
	enewgroup "device_management"
}
