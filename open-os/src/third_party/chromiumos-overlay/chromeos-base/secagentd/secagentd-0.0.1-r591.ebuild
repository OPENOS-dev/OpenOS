# Copyright 2022 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

EAPI=7
CROS_WORKON_COMMIT="82604baca99c4163a90a26ec5db16f3da23ffdef"
CROS_WORKON_TREE=("518b50f8b6d01e95cbd933487ed7c6452ac4acb3" "2e85ba6c8e700901ddd4762febc0d6f6a2c91726" "f91b6afd5f2ae04ee9a2c19109a3a4a36f7659e6")
CROS_WORKON_INCREMENTAL_BUILD="1"
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_LOCALNAME="platform2"
CROS_WORKON_DESTDIR="${S}/platform2"
CROS_WORKON_SUBTREE="common-mk secagentd .gn"

PLATFORM_SUBDIR="secagentd"

# Do not run test parallelly until unit tests are fixed.
# shellcheck disable=SC2034
PLATFORM_PARALLEL_GTEST_TEST="no"

inherit cros-workon platform cros-protobuf user

DESCRIPTION="Enterprise security event reporting."
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform2/+/HEAD/secagentd/"
LICENSE="BSD-Google"
KEYWORDS="*"
IUSE="+secagentd_min_core_btf proto_force_optimize_speed kernel-5_10 kernel-5_15"

COMMON_DEPEND="
	chromeos-base/attestation-client:=
	chromeos-base/cryptohome-client:=
	chromeos-base/dlp:=
	chromeos-base/featured:=
	chromeos-base/metrics:=
	chromeos-base/missive:=
	chromeos-base/session_manager-client:=
	chromeos-base/shill-dbus-client:=
	chromeos-base/shill-client:=
	chromeos-base/system_api:=
	chromeos-base/tpm_manager-client:=
	chromeos-base/vboot_reference:=
	dev-cpp/abseil-cpp:=
	dev-libs/openssl:0=
	dev-libs/re2:=
	>=dev-libs/libbpf-0.8.1
"

# RDepending on linux-sources makes it so that the min BTFs are rebuilt (if
# applicable) once when any kernel source is updated.
RDEPEND="${COMMON_DEPEND}
	>=sys-process/audit-3.0
	secagentd_min_core_btf? (
		virtual/linux-sources:=
	)
"

# Depending on linux-sources makes it so vmlinux is available on the board
# build root. This is needed so bpftool can generate vmlinux.h at build time.
DEPEND="${COMMON_DEPEND}
	chromeos-base/protofiles:=
	virtual/linux-sources:=
	virtual/pkgconfig:=
"

# bpftool is needed in the SDK to generate C code skeletons from compiled BPF applications.
# pahole is needed in the SDK to generate vmlinux.h and detached BTFs.
BDEPEND="
	dev-util/bpftool:=
	dev-util/pahole:=
"

pkg_setup() {
	enewuser "secagentd"
	enewgroup "secagentd"
	cros-workon_pkg_setup
}

src_install() {
	platform_src_install

	dosbin "${OUT}"/secagentd
	if use secagentd_min_core_btf; then
		insinto /usr/share/btf/secagentd
		doins "${OUT}"/gen/btf/*.min.btf
	fi
}

platform_pkg_test() {
	platform_test "run" "${OUT}/secagentd_testrunner"
}
