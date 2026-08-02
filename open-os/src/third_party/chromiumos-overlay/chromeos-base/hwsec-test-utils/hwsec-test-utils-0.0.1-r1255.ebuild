# Copyright 2020 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

EAPI=7

CROS_WORKON_COMMIT="5490ef0a16ebfd6671dbbbf8238f9e8785802036"
CROS_WORKON_TREE=("581dcc7d00cfed28430e5d4c88b43acee9f97050" "518b50f8b6d01e95cbd933487ed7c6452ac4acb3" "cece0820c8ff64cdc1af4fc76159fc7c1405e8ac" "6a9a79d4054ab44a049d1756dc7e3cce63bc2cf8" "b8c09b0737d26e92e8c1543f785a92a112de09cc" "6c758e6ee408412dc2833681b642c2d01bc2dab1" "f182925db23c77d96440e4dade69bf4225393f47" "f91b6afd5f2ae04ee9a2c19109a3a4a36f7659e6")
CROS_WORKON_INCREMENTAL_BUILD=1
CROS_WORKON_LOCALNAME="platform2"
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_OUTOFTREE_BUILD=1
CROS_WORKON_SUBTREE="attestation common-mk hwsec-test-utils libhwsec libhwsec-foundation tpm_manager trunks .gn"

PLATFORM_SUBDIR="hwsec-test-utils"

inherit cros-workon platform cros-protobuf

DESCRIPTION="Hwsec-related test-only features. This package resides in test images only."
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform2/+/HEAD/hwsec-test-utils/"

LICENSE="BSD-Google"
KEYWORDS="*"
IUSE="tpm tpm_dynamic tpm2"
REQUIRED_USE="
	tpm_dynamic? ( tpm tpm2 )
	!tpm_dynamic? ( ?? ( tpm tpm2 ) )
"

RDEPEND="
	tpm? (
		app-crypt/trousers:=
	)
	chromeos-base/attestation:=
	chromeos-base/libhwsec:=
	chromeos-base/libhwsec-foundation:=
	chromeos-base/system_api:=
	chromeos-base/tpm_manager-client:=
	tpm2? (
		chromeos-base/trunks:=
	)
	dev-libs/openssl:=
"

DEPEND="${RDEPEND}
	tpm2? (
		chromeos-base/trunks:=
	)
"
