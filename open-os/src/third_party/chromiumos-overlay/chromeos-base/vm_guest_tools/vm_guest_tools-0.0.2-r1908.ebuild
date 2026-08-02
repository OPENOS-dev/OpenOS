# Copyright 2017 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_WORKON_COMMIT="07ce9deec28cdb32cf2f2f5083871bec301123e1"
CROS_WORKON_TREE=("518b50f8b6d01e95cbd933487ed7c6452ac4acb3" "f91b6afd5f2ae04ee9a2c19109a3a4a36f7659e6" "99ccc7ec4af45030cf69f7e3d92ff1d2f8c13b4c" "1a926ed7365dbee54e3d3fa53f0a9a37f3f1c60a" "b345628a056229b5c6c17b7f909f7622e7edb594" "f900387945e5be1d73ed6d9e875d8b7725ee680e" "a250b39ebcbea9c5529b65b1a28c5ba8452e3d14" "7a15a8b912277b0960b1b6883e14ca1a0e0cd7f6" "8dbc0c4897e95790b9bd6e583e2a517abdb136a9" "3c9b40e3d1ef1ab48e39bea628c69e73526a6f56" "a3e5fad0dccbf2f38ee102768dca68c02c6805f4" "6a89f6f751071a9cb483091db9b19d0cd2188bb1" "cbef4aa32160ef6f4e0c92f1d8382acba6812ef1" "58a825ec3ca1f8ec55597e4a83f9068a5037d8d8" "ebfa9f3b862121172b363d28c3021726c6612560" "7ebc9324cd96c81d465e681ce3161b823fd66eee" "a01dc69a1e1fa54805fe9b48ce5c278a7e70de0c")
CROS_WORKON_LOCALNAME="platform2"
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_OUTOFTREE_BUILD=1
CROS_WORKON_INCREMENTAL_BUILD=1

PLATFORM2_PATHS=(
	common-mk
	.gn

	vm_tools/BUILD.gn
	vm_tools/guest
	vm_tools/common

	vm_tools/demos
	vm_tools/garcon
	vm_tools/guest_service_failure_notifier
	vm_tools/maitred
	vm_tools/notificationd
	vm_tools/port_listener
	vm_tools/sommelier
	vm_tools/syslog
	vm_tools/vsh

	# Required by the fuzzer
	vm_tools/DIR_METADATA
	vm_tools/OWNERS
	vm_tools/testdata
)
CROS_WORKON_SUBTREE="${PLATFORM2_PATHS[*]}"

PLATFORM_SUBDIR="vm_tools"

inherit cros-go cros-workon platform cros-protobuf user

DESCRIPTION="VM guest tools for Chrome OS"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform2/+/HEAD/vm_tools"

LICENSE="BSD-Google"
KEYWORDS="*"
IUSE="kvm_guest vm-containers fuzzer vm_borealis"

# This ebuild should only be used on VM guest boards.
REQUIRED_USE="kvm_guest"

COMMON_DEPEND="
	!!chromeos-base/vm_tools
	chromeos-base/minijail:=
	net-libs/grpc:=
	dev-cpp/abseil-cpp:=
	dev-go/protobuf-legacy-api:=
	x11-libs/libX11:=
	dev-libs/wayland:=
	dev-libs/libbpf:=
	dev-libs/re2:=
"

RDEPEND="
	${COMMON_DEPEND}
	vm-containers? (
		chromeos-base/crash-reporter
		chromeos-base/crostini-metric-reporter
	)
	!fuzzer? (
		chromeos-base/sommelier
	)
"

DEPEND="
	${COMMON_DEPEND}
	dev-go/grpc:=
	dev-go/protobuf:=
	sys-kernel/linux-headers:=
	chromeos-base/system_api:=
	chromeos-base/vm_protos:=
	virtual/linux-sources:=
"

BDEPEND="
	dev-util/bpftool:=
	dev-util/pahole:=
"

src_install() {
	platform_src_install

	dobin "${OUT}"/vm_syslog
	dosbin "${OUT}"/vshd

	if use vm-containers || use vm_borealis; then
		dobin "${OUT}"/garcon
	fi
	if use vm-containers; then
		dobin "${OUT}"/guest_service_failure_notifier
		dobin "${OUT}"/notificationd
		dobin "${OUT}"/port_listener
		dobin "${OUT}"/wayland_demo
		dobin "${OUT}"/x11_demo
	fi

	# fuzzer_component_id is unknown/unlisted
	platform_fuzzer_install "${S}"/OWNERS "${OUT}"/garcon_desktop_file_fuzzer \
		--dict "${S}"/testdata/garcon_desktop_file_fuzzer.dict
	platform_fuzzer_install "${S}"/OWNERS "${OUT}"/garcon_icon_index_file_fuzzer \
		--dict "${S}"/testdata/garcon_icon_index_file_fuzzer.dict
	platform_fuzzer_install "${S}"/OWNERS "${OUT}"/garcon_ini_parse_util_fuzzer
	platform_fuzzer_install "${S}"/OWNERS "${OUT}"/garcon_mime_types_parser_fuzzer

	dobin "${OUT}"/maitred
	dosym /usr/bin/maitred /sbin/init

	# Create a folder for process configs to be launched at VM startup.
	dodir /etc/maitred/

	use fuzzer || dosym /run/resolv.conf /etc/resolv.conf

	CROS_GO_WORKSPACE="${OUT}/gen/go"
	cros-go_src_install
}

platform_pkg_test() {
	local tests=(
		maitred_init_test
		maitred_service_test
		maitred_syslog_test
	)

	local container_tests=(
		garcon_desktop_file_test
		garcon_icon_index_file_test
		garcon_icon_finder_test
		garcon_mime_types_parser_test
		notificationd_test
	)

	if use vm-containers || use vm_borealis; then
		tests+=( "${container_tests[@]}" )
	fi

	local test_bin
	for test_bin in "${tests[@]}"; do
		platform_test "run" "${OUT}/${test_bin}"
	done
}

pkg_preinst() {
	# We need the syslog user and group for both host and guest builds.
	enewuser syslog
	enewgroup syslog
}
