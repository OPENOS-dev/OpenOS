# Copyright 2018 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_WORKON_COMMIT="5490ef0a16ebfd6671dbbbf8238f9e8785802036"
CROS_WORKON_TREE=("518b50f8b6d01e95cbd933487ed7c6452ac4acb3" "5ff51a7a7a64def8f4a5733260f0da87c7f8133f" "6a9a79d4054ab44a049d1756dc7e3cce63bc2cf8" "b8c09b0737d26e92e8c1543f785a92a112de09cc" "f91b6afd5f2ae04ee9a2c19109a3a4a36f7659e6")
PYTHON_COMPAT=( python3_11 )

CROS_WORKON_LOCALNAME="platform2"
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_DESTDIR="${S}/platform2"
CROS_WORKON_INCREMENTAL_BUILD=1
# TODO(crbug.com/809389): Avoid directly including headers from other packages.
CROS_WORKON_SUBTREE="common-mk cryptohome libhwsec libhwsec-foundation .gn"

PLATFORM_SUBDIR="cryptohome/dev-utils"

inherit python-any-r1 cros-workon platform cros-protobuf

DESCRIPTION="Cryptohome developer and testing utilities for Chromium OS"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform2/+/HEAD/cryptohome"

LICENSE="BSD-Google"
SLOT="0/0"
KEYWORDS="*"
IUSE="tpm tpm_dynamic tpm_insecure_fallback tpm2"
# This package has no unittests.
RESTRICT="test"

REQUIRED_USE="
	tpm_dynamic? ( tpm tpm2 )
	!tpm_dynamic? ( ?? ( tpm tpm2 ) )
"

COMMON_DEPEND="
	tpm? (
		app-crypt/trousers:=
	)
	tpm2? (
		chromeos-base/trunks:=
	)
	chromeos-base/attestation:=
	chromeos-base/biod_proxy:=
	chromeos-base/bootlockbox-client:=
	chromeos-base/cbor:=
	chromeos-base/chaps:=
	chromeos-base/chromeos-config-tools:=
	chromeos-base/cryptohome:=
	chromeos-base/cryptohome-client:=
	chromeos-base/featured:=
	chromeos-base/libcrossystem:=
	chromeos-base/libstorage:=
	chromeos-base/libhwsec:=
	chromeos-base/libhwsec-foundation:=
	>=chromeos-base/metrics-0.0.1-r3152:=
	chromeos-base/system_api:=
	chromeos-base/secure-erase-file:=
	dev-cpp/abseil-cpp:=
	dev-libs/flatbuffers:=
	dev-libs/glib:=
	dev-libs/openssl:=
	sys-apps/keyutils:=
	sys-fs/e2fsprogs:=
	sys-fs/ecryptfs-utils:=
"

RDEPEND="${COMMON_DEPEND}
	chromeos-base/tpm_manager:=
"

DEPEND="${COMMON_DEPEND}
	chromeos-base/device_management-client:=
	chromeos-base/vboot_reference:=
"

# shellcheck disable=SC2016
BDEPEND="
	chromeos-base/chromeos-dbus-bindings
	dev-libs/flatbuffers
	$(python_gen_any_dep '
		dev-python/jinja2[${PYTHON_USEDEP}]
		dev-python/flatbuffers[${PYTHON_USEDEP}]
	')
"

python_check_deps() {
	python_has_version -b "dev-python/flatbuffers[${PYTHON_USEDEP}]"
}
