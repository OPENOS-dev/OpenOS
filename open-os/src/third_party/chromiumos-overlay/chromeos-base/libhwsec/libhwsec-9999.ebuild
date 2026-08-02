# Copyright 2019 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

EAPI=7

PYTHON_COMPAT=( python3_11 )

CROS_WORKON_INCREMENTAL_BUILD=1
CROS_WORKON_LOCALNAME="platform2"
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_OUTOFTREE_BUILD=1
CROS_WORKON_SUBTREE="common-mk libcrossystem libhwsec libhwsec-foundation libstorage metrics tpm_manager tpm2-simulator trunks .gn"

PLATFORM_SUBDIR="libhwsec"

inherit python-any-r1 cros-workon platform cros-protobuf

DESCRIPTION="Crypto and utility functions used in TPM related daemons."
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform2/+/HEAD/libhwsec/"

LICENSE="BSD-Google"
KEYWORDS="~*"
IUSE="profiling test fuzzer tpm tpm2 tpm_dynamic cr50_onboard ti50_onboard"

COMMON_DEPEND="
	chromeos-base/chromeos-ec-headers:=
	chromeos-base/libbrillo:=[fuzzer?]
	chromeos-base/libcrossystem:=
	chromeos-base/libhwsec-foundation:=
	chromeos-base/libstorage:=
	chromeos-base/metrics:=
	chromeos-base/system_api:=
	chromeos-base/tpm_manager-client:=
	dev-cpp/abseil-cpp:=
	dev-libs/openssl:0=
	dev-libs/flatbuffers:=
	dev-libs/re2:=
	tpm2? (
		chromeos-base/pinweaver:=
		chromeos-base/trunks:=
	)
	tpm? ( app-crypt/trousers:= )
	fuzzer? (
		app-crypt/trousers:=
		chromeos-base/trunks:=
	)
	test? (
		app-crypt/trousers:=
		chromeos-base/pinweaver:=
		chromeos-base/trunks:=
		chromeos-base/tpm2-simulator:=[test]
	)
"

RDEPEND="${COMMON_DEPEND}"
DEPEND="${COMMON_DEPEND}"

# shellcheck disable=SC2016
BDEPEND="
	dev-libs/flatbuffers
	$(python_gen_any_dep '
		dev-python/jinja2[${PYTHON_USEDEP}]
		dev-python/flatbuffers[${PYTHON_USEDEP}]
	')
"

python_check_deps() {
	python_has_version -b "dev-python/flatbuffers[${PYTHON_USEDEP}]"
}

src_install() {
	platform_src_install

	local fuzzer_component_id="1188704"

	platform_fuzzer_install "${S}"/OWNERS \
		"${OUT}"/libhwsec_tpm1_cmk_migration_parser_fuzzer \
		--comp "${fuzzer_component_id}"

	platform_fuzzer_install "${S}"/OWNERS \
		"${OUT}"/libhwsec_tpm2_backend_fuzzer \
		--comp "${fuzzer_component_id}" \
		--dict "${S}"/fuzzers/testdata/tpm2_commands.dict
}
