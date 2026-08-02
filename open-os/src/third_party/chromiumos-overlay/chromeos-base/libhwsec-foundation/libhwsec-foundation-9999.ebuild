# Copyright 2021 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

EAPI=7

CROS_WORKON_INCREMENTAL_BUILD=1
CROS_WORKON_LOCALNAME="platform2"
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_OUTOFTREE_BUILD=1
CROS_WORKON_SUBTREE="common-mk metrics libcrossystem libhwsec-foundation .gn"

PLATFORM_SUBDIR="libhwsec-foundation"

# This doesn't depend on protobuf directly, but needs to be rebuilt when
# protobuf is upgraded so include protobuf as a dependnecy through
# cros-protobuf.
inherit cros-protobuf cros-workon platform tmpfiles

DESCRIPTION="Crypto and utility functions used in TPM related daemons."
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform2/+/HEAD/libhwsec-foundation/"

LICENSE="BSD-Google"
KEYWORDS="~*"
IUSE="cros_host profiling test tpm tpm_dynamic tpm2 tpm2_simulator"

DEPEND="
	!cros_host? (
		chromeos-base/attestation-client:=
		chromeos-base/cryptohome-client:=
		chromeos-base/device_management-client:=
		chromeos-base/tpm_manager-client:=
	)
	>=chromeos-base/metrics-0.0.1-r3152
	chromeos-base/libcrossystem:=
	chromeos-base/libbrillo:=[fuzzer?]
	chromeos-base/system_api:=
	chromeos-base/tpm_manager-client:=
	chromeos-base/vboot_reference:=
	dev-libs/openssl:=
	dev-libs/re2:=
	"

RDEPEND="${DEPEND}"

src_install() {
	platform_src_install

	# Install tmpfiles.d for creating dir for profiling data.
	if use profiling; then
		dotmpfiles profiling/tmpfiles.d/profiling.conf
	fi

	local fuzzer_component_id="1188704"
	platform_fuzzer_install "${S}"/OWNERS \
		"${OUT}"/libhwsec_foundation_rsa_oaep_decrypt_fuzzer \
		--comp "${fuzzer_component_id}" \
		fuzzers/testdata/*
}
