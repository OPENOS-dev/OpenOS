# Copyright 2014 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_WORKON_COMMIT="aea6144b50d3d21b5fecd2745f9bb6c65d7f7b90"
CROS_WORKON_TREE=("518b50f8b6d01e95cbd933487ed7c6452ac4acb3" "fc38f33ebe784740abd46cdf82c71e20ab875778" "b8c09b0737d26e92e8c1543f785a92a112de09cc" "f91b6afd5f2ae04ee9a2c19109a3a4a36f7659e6")
CROS_WORKON_LOCALNAME="platform2"
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_OUTOFTREE_BUILD=1
CROS_WORKON_SUBTREE="common-mk tpm2-simulator libhwsec-foundation .gn"

PLATFORM_SUBDIR="tpm2-simulator"

inherit cros-workon platform tmpfiles user

DESCRIPTION="TPM 2.0 Simulator"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform2/+/HEAD/tpm2-simulator/"

LICENSE="BSD-Google"
KEYWORDS="*"

IUSE="selinux ti50_onboard tpm tpm2 test tpm2_simulator tpm2_simulator_manufacturer"

COMMON_DEPEND="
	tpm? ( !test? ( dev-libs/libtpms:= ) )
	tpm2? (
		chromeos-base/tpm2:=[tpm2_simulator?]
		chromeos-base/tpm2:=[tpm2_simulator_manufacturer?]
	)
	test? ( chromeos-base/tpm2:= )
	chromeos-base/libhwsec-foundation:=
	chromeos-base/minijail:=
	chromeos-base/pinweaver:=
	ti50_onboard? ( !test? ( chromeos-base/ti50-emulator:= ) )
	chromeos-base/vboot_reference:=[tpm2_simulator?]
	dev-libs/openssl:0=
	sys-libs/libselinux:=
	"

RDEPEND="
	${COMMON_DEPEND}
	selinux? (
		chromeos-base/selinux-policy
	)
"
DEPEND="${COMMON_DEPEND}"

BDEPEND="
	chromeos-base/minijail
"

pkg_preinst() {
	if use tpm2_simulator; then
		enewuser tpm2-simulator
		enewgroup tpm2-simulator
	fi
}
