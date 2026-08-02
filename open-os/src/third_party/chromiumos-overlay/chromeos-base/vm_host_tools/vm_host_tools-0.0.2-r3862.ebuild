#-Copyright 2017 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_WORKON_COMMIT="c5febd36d0ae5294cd465148a992962eadca7edc"
CROS_WORKON_TREE=("518b50f8b6d01e95cbd933487ed7c6452ac4acb3" "b8033e453c7d9518619e90fb100d7d90d7b4026d" "3d01071ecc84cb5a5781cd4a17dc0efe279a6169" "9576f262a57fd7a5bff4991afa8dcb9c6650c635" "f91b6afd5f2ae04ee9a2c19109a3a4a36f7659e6" "f8c33d450bd37923ca39cdffe1f0d23740d27659" "8f43dfd7edbac2a1aa65be54dfb9e8a923f456dd" "99ccc7ec4af45030cf69f7e3d92ff1d2f8c13b4c" "be4daf2fae9ad9c79c2c1b6c42896e56c19dab46" "b345628a056229b5c6c17b7f909f7622e7edb594" "f0e000e25e75624bdb59d01e39f3e00da693191a" "9b6bc46cbd519d8b3059876a2a7ba763aba5a20a" "3fad3ffa17926c5fd6a63889ff04354dc6a3d938" "ab72a5b1472cd0d142a4e2cb34e4b128ef5804c1" "0bd2c2352a9af141aae80780b2af7e1f0c668174" "8dbc0c4897e95790b9bd6e583e2a517abdb136a9" "4f9e997c7cfc37f43a50de4fe1749cc5c93fa102" "10462a066f83196231e0a49c5743432976f2271b" "980f6d6e1b72884f7d195b82e68868d11c7df8d6" "cbef4aa32160ef6f4e0c92f1d8382acba6812ef1" "672f03e167a82252e86cb7f55803d5899d8fa7b8" "0c4a08a4f57b5d6806df4ad5cec9d2c8fb02fab0" "694537d728a9bcf9a8f3e29a3bcb2d9d9cf61c7a" "58a825ec3ca1f8ec55597e4a83f9068a5037d8d8" "ebfa9f3b862121172b363d28c3021726c6612560" "7ebc9324cd96c81d465e681ce3161b823fd66eee" "16839d1d0fcecb39e52d1bc3a65aa02d62c1ee6d")
CROS_WORKON_LOCALNAME="platform2"
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_OUTOFTREE_BUILD=1
CROS_WORKON_INCREMENTAL_BUILD=1

PLATFORM2_PATHS=(
	common-mk
	featured
	metrics
	net-base
	.gn
	spaced
	libcrossystem

	vm_tools/BUILD.gn
	vm_tools/host
	vm_tools/common

	vm_tools/cicerone
	vm_tools/concierge
	vm_tools/dbus_bindings
	vm_tools/dbus
	vm_tools/init
	vm_tools/maitred
	vm_tools/modprobe
	vm_tools/pstore_dump
	vm_tools/seneschal
	vm_tools/syslog
	vm_tools/tmpfiles.d
	vm_tools/udev
	vm_tools/vhost_user_starter
	vm_tools/vsh

	# Required by the fuzzer
	vm_tools/DIR_METADATA
	vm_tools/OWNERS

	# Required by the vm_concierge
	chromeos-config
)
CROS_WORKON_SUBTREE="${PLATFORM2_PATHS[*]}"

PLATFORM_SUBDIR="vm_tools"
# Do not run test parallelly until unit tests are fixed.
# shellcheck disable=SC2034
PLATFORM_PARALLEL_GTEST_TEST="no"

inherit tmpfiles cros-workon platform cros-protobuf udev user arc-build-constants

DESCRIPTION="VM host tools for Chrome OS"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform2/+/HEAD/vm_tools"

LICENSE="BSD-Google"
KEYWORDS="*"
IUSE="
	arcvm_gki
	borealis_host
	cross_domain_context
	cross_domain_context_borealis
	+crosvm-fixed-blob-mapping
	crosvm_limit_armv8pmu_counters
	+crosvm-virtio-video
	crosvm-virtio-video-vd
	fuzzer
	+kvm_host
	libglvnd
	+seccomp
	venus_gwp_asan
	vfio_gpu
	virtgpu_native_context
	vulkan
	wilco
"
REQUIRED_USE="kvm_host"

COMMON_DEPEND="
	app-arch/libarchive:=[zstd]
	!!chromeos-base/vm_tools
	chromeos-base/chunnel:=
	chromeos-base/chromeos-config-tools:=
	chromeos-base/crosvm:=
	chromeos-base/libcrossystem:=
	>=chromeos-base/metrics-0.0.1-r3617:=
	chromeos-base/minijail:=
	chromeos-base/net-base:=
	chromeos-base/patchpanel-client:=
	chromeos-base/perfetto:=
	chromeos-base/spaced
	dev-cpp/abseil-cpp:=
	dev-libs/re2:=
	net-libs/grpc:=
	sys-apps/util-linux:=
"

RDEPEND="
	${COMMON_DEPEND}
	app-arch/zstd:=
	dev-rust/s9
	borealis_host? ( chromeos-base/shadercached:= )
"
DEPEND="
	${COMMON_DEPEND}
	chromeos-base/dlcservice-client:=
	chromeos-base/featured:=
	chromeos-base/session_manager-client:=
	chromeos-base/shill-client:=
	chromeos-base/system_api:=[fuzzer?]
	chromeos-base/vboot_reference:=
	chromeos-base/vm_protos:=
	fuzzer? ( dev-libs/libprotobuf-mutator:= )
	sys-fs/udev:=
"

BDEPEND="
	chromeos-base/chromeos-dbus-bindings
	chromeos-base/minijail
"

get_vmlog_forwarder_start_services() {
	local start_services="starting vm_concierge"
	echo "${start_services}"
}

get_vmlog_forwarder_stop_services() {
	local stop_services="stopped vm_concierge"
	if use wilco; then
		stop_services="stopping system-services"
	fi
	echo "${stop_services}"
}

pkg_setup() {
	# Duplicated from the crosvm ebuild. These are necessary here in order
	# to create the daemon-store folder for concierge in src_install().
	enewuser crosvm
	enewgroup crosvm
	enewuser pluginvm
	cros-workon_pkg_setup

	enewuser crosvm-root
	enewgroup crosvm-root
}

src_configure() {
	platform_src_configure

	if use test; then
		# Do not check odr violations in address sanitizer.
		export ASAN_OPTIONS+=":detect_odr_violation=0:"
	fi
}

src_install() {
	platform_src_install

	dobin "${OUT}"/cicerone_client
	dobin "${OUT}"/maitred_client
	dobin "${OUT}"/seneschal
	dobin "${OUT}"/seneschal_client
	dobin "${OUT}"/vm_cicerone
	dobin "${OUT}"/vm_concierge
	dobin "${OUT}"/vmlog_forwarder
	dobin "${OUT}"/vsh

	if use arcvm; then
		dobin "${OUT}"/vm_pstore_dump
		dobin "${OUT}"/vshd
	fi

	# fuzzer_component_id is unknown/unlisted
	platform_fuzzer_install "${S}"/OWNERS "${OUT}"/cicerone_container_listener_fuzzer
	platform_fuzzer_install "${S}"/OWNERS "${OUT}"/vsh_client_fuzzer

	# Install header for passing USB devices to plugin VMs.
	insinto /usr/include/vm_concierge
	doins concierge/plugin_vm_usb.h

	insinto /etc/init
	doins init/seneschal.conf
	# TODO(b/288998343): remove when bug is fixed and interrupted discards are
	# not lost.
	doins init/trim_filesystem.conf
	doins init/vm_cicerone.conf
	doins init/vm_concierge.conf

	# TODO(b/353431869): remove when the finch experiment is finished.
	doins init/vm_concierge_nodefer.conf
	exeinto /usr/share/cros/init/
	doexe init/check_defer_concierge_config.sh

	dotmpfiles tmpfiles.d/*.conf

	# Modify vmlog_forwarder starting and stopping conditions based on USE flags.
	sed \
		"-e s,@dependent_start_services@,$(get_vmlog_forwarder_start_services),"\
		"-e s,@dependent_stop_services@,$(get_vmlog_forwarder_stop_services)," \
		init/vmlog_forwarder.conf.in | newins - vmlog_forwarder.conf

	insinto /etc/dbus-1/system.d
	doins dbus/*.conf

	if use vfio_gpu; then
		insinto /etc/modprobe.d
		doins modprobe/vfio-dgpu.conf

		exeinto /sbin
		doexe modprobe/dgpu.sh

		# Udev rules to bind dGPU to different modules.
		udev_dorules udev/45-vfio-dgpu.rules
	fi

	insinto /usr/local/vms/etc
	doins concierge/config/arcvm_dev.conf

	insinto /usr/share/policy
	if use seccomp; then
		newins "init/vm_cicerone-seccomp-${ARCH}.policy" vm_cicerone-seccomp.policy
	fi

	udev_dorules udev/99-vm.rules

	keepdir /opt/google/vms

	# Create daemon store folder for crosvm and pvm
	local crosvm_store="/etc/daemon-store/crosvm"
	dodir "${crosvm_store}"
	fperms 0750 "${crosvm_store}"
	fowners crosvm:crosvm "${crosvm_store}"

	local pvm_store="/etc/daemon-store/pvm"
	dodir "${pvm_store}"
	fperms 0770 "${pvm_store}"
	fowners pluginvm:crosvm "${pvm_store}"
}

platform_pkg_test() {
	local tests=(
		cicerone_test
		concierge_test
		syslog_forwarder_test
		vsh_test
	)
	if use arcvm; then
		tests+=(
			vm_pstore_dump_test
		)
	fi

	# Running a gRPC server under qemu-user causes flake, at least with the
	# combination of gRPC 1.16.1 and qemu 3.0.0. Disable TerminaVmTest.* while
	# running under qemu to avoid triggering this flake.
	# TODO(crbug.com/1066425): Reenable gRPC server tests under qemu-user.
	local qemu_gtest_filter="-TerminaVmTest.*"
	local test_bin
	for test_bin in "${tests[@]}"; do
		platform_test "run" "${OUT}/${test_bin}" "0" "" "${qemu_gtest_filter}"
	done
}

pkg_preinst() {
	# We need the syslog user and group for both host and guest builds.
	enewuser syslog
	enewgroup syslog

	enewuser vm_cicerone
	enewgroup vm_cicerone

	enewuser seneschal
	enewgroup seneschal
	enewuser seneschal-dbus
	enewgroup seneschal-dbus

	enewuser pluginvm
	enewgroup pluginvm

	enewgroup virtaccess
}
