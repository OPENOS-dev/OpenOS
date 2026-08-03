# Copyright 2021 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI="7"

CROS_WORKON_COMMIT=("5490ef0a16ebfd6671dbbbf8238f9e8785802036" "85ccfbdb4f09b90bc1d7171ac77c979e9b299e2e" "571771cc4700e5af37aaf5fbdd0a01192facf224")
CROS_WORKON_TREE=("518b50f8b6d01e95cbd933487ed7c6452ac4acb3" "f91b6afd5f2ae04ee9a2c19109a3a4a36f7659e6" "37f289ae885c2556126b82a0b3804b5c4e3be858" "7edadb55afb89f5a50675ece1652243c9ee8138a")
CROS_WORKON_PROJECT=(
	"chromiumos/platform2"
	"aosp/platform/packages/modules/Bluetooth"
	"aosp/platform/frameworks/proto_logging"
)
CROS_WORKON_LOCALNAME=(
	"../platform2"
	"../aosp/packages/modules/Bluetooth/local"
	"../aosp/frameworks/proto_logging/local"
)
CROS_WORKON_DESTDIR=(
	"${S}/platform2"
	"${S}/platform2/bt"
	"${S}/platform2/external/proto_logging"
)
CROS_WORKON_SUBTREE=("common-mk .gn" "" "")
CROS_WORKON_EGIT_BRANCH=("main" "main" "master")
CROS_WORKON_OPTIONAL_CHECKOUT=("" "" "")
CROS_WORKON_INCREMENTAL_BUILD=1
PLATFORM_SUBDIR="bt"

# floss_strict: Treat warnings as errors
IUSE="bt_dynlib floss_upstream floss_strict bt_nonstandard_codecs flex_bluetooth"
# floss_rootcanal: build the floss with special HCIHAL to use the Rootcanal.
IUSE+=" floss_rootcanal"
# bluetooth_chip_*: Indicate the controller model so we can install the
#                   corresponding qurik for it.
IUSE+=" bluetooth_chip_ac7265 bluetooth_chip_mvl8897"

# TODO(b/331777962): This should be possible to dynamically link with libstd,
# but doesn't work out of the box.
# This is used by cros-rust.eclass, so disable 'unused' complaints
# shellcheck disable=SC2034
CROS_RUST_FORCE_STATIC_LINK=1

inherit cros-workon cros-toolchain-funcs cros-rust platform cros-protobuf tmpfiles user udev

# TODO(b/324584880): Stop linking against libstdc++.
inherit cros-gcc

DESCRIPTION="Bluetooth Tools and System Daemons for Linux"
HOMEPAGE="https://android.googlesource.com/platform/packages/modules/Bluetooth"

# Apache-2.0 for system/bt
LICENSE="Apache-2.0"

KEYWORDS="*"
#
# TODO(b/188819708)
# Floss continues to depend on bluez for a few things:
#  - Several headers (bluetooth.h, l2cap.h, etc) which are used by Chrome
#  - Bluetooth user + group are added in bluez's postinst
#
DEPEND="
	dev-rust/third-party-crates-src:=
	dev-rust/featured:=
	chromeos-base/featured:=
	chromeos-base/metrics:=
	chromeos-base/system_api:=
	dev-libs/libfmt:=
	dev-libs/flatbuffers:=
	dev-libs/modp_b64:=
	dev-libs/tinyxml2:=
	dev-libs/openssl:=
	net-wireless/libbluez:=
	media-sound/liblc3
	sys-apps/dbus:=
"

BDEPEND="
	chromeos-base/libchrome
	chromeos-base/minijail
	dev-cpp/gtest
	dev-libs/flatbuffers
	dev-libs/tinyxml2:=
	dev-util/cmake
	dev-util/cxxbridge-cmd
	dev-util/pdl-compiler
	dev-rust/grpcio-compiler:=
	net-wireless/aconfig:=
	net-wireless/sysprop_cpp:=
	sys-devel/bison
	sys-devel/flex
"

RDEPEND="
	${DEPEND}
	flex_bluetooth? ( chromeos-base/flex_bluetooth )
"

DOCS=( README.md )

# Patches applied on the local folder for Floss launch.
# UPSTREAM patches MUST be removed at next branch point,
# and CHROMIUM patches will be removed in the future.
# DO NOT modify the "REL_PATCHES=" and the "# REL_PATCHES end" lines,
# otherwise some scripts may break.
REL_PATCHES=(
	"${FILESDIR}"/patches/0001-CHROMIUM-Add-sco-quirk-for-pixel-buds-pro.patch
	"${FILESDIR}"/patches/0002-CHROMIUM-Add-stack-config-to-exclude-HSP-in-SDP.patch
	"${FILESDIR}"/patches/0003-CHROMIUM-Reject-incoming-bond-if-already-bonded.patch
	"${FILESDIR}"/patches/0004-CHROMIUM-gd-Don-t-always-assert-on-unexpected-events.patch
	"${FILESDIR}"/patches/0005-CHROMIUM-Don-t-reject-IOCap-request-if-bonding-is-te.patch
	"${FILESDIR}"/patches/0006-CHROMIUM-GD-Check-opcode-support-before-reading-LE-f.patch
	"${FILESDIR}"/patches/0007-CHROMIUM-controller-config-for-boundary-sniff-mode.patch
	"${FILESDIR}"/patches/0008-CHROMIUM-start-discovery-for-a-while-after-resume.patch
	"${FILESDIR}"/patches/0009-CHROMIUM-a2dp-Avoid-touching-WeakPtr-on-another-thre.patch
	"${FILESDIR}"/patches/1001-UPSTREAM-Wire-BT_PROPERTY_UUIDS_LE-and-convert-it-to.patch
	"${FILESDIR}"/patches/1002-UPSTREAM-Add-flag-nrpa_for_non_connectable_adv.patch
	"${FILESDIR}"/patches/1003-UPSTREAM-Replace-nrpa_non_connectable_adv-with-nrpa_.patch
	"${FILESDIR}"/patches/1004-UPSTREAM-Remove-flag-nrpa_non_connectable_adv.patch
	"${FILESDIR}"/patches/1005-UPSTREAM-Handle-invalid-C-string-device-name-in-the-.patch
	"${FILESDIR}"/patches/1006-UPSTREAM-Handle-empty-length-advertisement-flag.patch
	"${FILESDIR}"/patches/1007-UPSTREAM-Fix-the-prop-len-of-BT_PROPERTY_ADAPTER_BON.patch
	"${FILESDIR}"/patches/1008-UPSTREAM-Fix-the-format-of-AdapterBondedDevices.patch
	"${FILESDIR}"/patches/1009-UPSTREAM-Pass-Bus-Options-by-value-with-std-move.patch
	"${FILESDIR}"/patches/1010-UPSTREAM-Add-include-for-base-NullCallback.patch
	"${FILESDIR}"/patches/1011-UPSTREAM-Support-C-23-build.patch
	"${FILESDIR}"/patches/1012-BACKPORT-Init-Aflags-in-bluetooth_init.patch
	"${FILESDIR}"/patches/1013-UPSTREAM-Add-check_set_event_mask_p2_support_before_.patch
	"${FILESDIR}"/patches/1014-BACKPORT-Send-set-event-mask-page-2-only-only-if-sup.patch
	"${FILESDIR}"/patches/1015-UPSTREAM-Force-immediate-termination-on-controller-i.patch
	"${FILESDIR}"/patches/1016-UPSTREAM-Fix-acl_arbiter-missing-mutex-header.patch
	"${FILESDIR}"/patches/1017-BACKPORT-Switch-to-C-23-and-fix-compiler-error.patch
	"${FILESDIR}"/patches/1018-UPSTREAM-More-aggresively-connect-profiles-on-outgoi.patch
	"${FILESDIR}"/patches/1019-UPSTREAM-Add-stubbed-audio_delay_reported_cb-callbac.patch
	"${FILESDIR}"/patches/1020-BACKPORT-Remove-report_eir_uuids-quirk-and-use-the-n.patch
	"${FILESDIR}"/patches/1021-UPSTREAM-Replace-base-ByteSwap-with-std-byteswap.patch
	"${FILESDIR}"/patches/1022-UPSTREAM-Disallow-disabling-Floss-via-btclient.patch
)  # REL_PATCHES end

src_unpack() {
	platform_src_unpack
	# Cros rust unpack should come after platform unpack otherwise platform
	# unpack will fail.
	cros-rust_src_unpack
}

src_prepare() {
	default

	if use floss_upstream; then
		die "floss must be built without USE=floss_upstream. Do you attempt to build floss-upstream?"
	fi
	local patch
	for patch in "${REL_PATCHES[@]}"; do
		eapply -F0 "${patch}"
	done

	cros-rust_src_prepare
}

build_host_tools() {
	tc-env_build platform "configure" "--host"
	tc-env_build platform "compile" "tools" "--host"

	# shellcheck disable=SC2154 # ECARGO_HOME is defined in cros-rust.eclass
	local rust_dir="${ECARGO_HOME}/bin"
	local cxx_outdir="$(cros-workon_get_build_dir)/out/Default"

	mkdir -p "${rust_dir}"
	cp "${cxx_outdir}/bluetooth_packetgen" "${rust_dir}/"
}

src_configure() {
	build_host_tools

	local cxx_outdir="$(cros-workon_get_build_dir)/out/Default"
	local rustflags=(
		# Add C/C++ build path to linker search path
		"-L ${cxx_outdir}"

		# Add sysroot libdir to search path.
		"-L ${SYSROOT}/usr/$(get_libdir)/"

		# Also ignore multiple definitions for now (added due to some
		# shared library shenaningans)
		"-C link-arg=-Wl,--allow-multiple-definition"

		# Allow some warnings from generated code
		"-A improper_ctypes_definitions -A improper_ctypes -A unknown_lints"
	)

	local bindgen_extra_clang_args=(
		# Set sysroot so it looks in correct path
		"--sysroot=${SYSROOT}"
	)

	# Treat warnings as errors if enabled.
	use floss_strict && rustflags+=( '--deny warnings' )

	# When using clang + asan, we need to link C++ lib. The build defaults
	# to using -lstdc++ which fails to link.
	use asan && rustflags+=( '-lc++' )

	export EXTRA_RUSTFLAGS="${rustflags[*]}"
	export TARGET_OS_VARIANT="chromeos"
	export BINDGEN_EXTRA_CLANG_ARGS="${bindgen_extra_clang_args[*]}"

	# TODO(b:316035779): Reenable -Wvla-cxx-extension and -Wdeprecated-this-capture.
	append-cxxflags "-Wno-vla-cxx-extension"
	append-cxxflags "-Wno-deprecated-this-capture"

	cros-rust_src_configure
	platform_src_configure "--target_os=chromeos"
}

floss_build_rust() {
	# Check if cxxflags has -fno-exceptions and set -DRUST_CXX_NO_EXCEPTIONS
	# This is required to build the cxx rust dependency
	if is-flagq -fno-exceptions; then
		append-cxxflags -DRUST_CXX_NO_EXCEPTIONS
	fi

	# cc rust package requires CLANG_PATH so it uses correct clang triple
	export CLANG_PATH="$(tc-getCC)"
	# shellcheck disable=SC2154 # BUILD_CFLAGS is defined in
	# toolchain-funcs.eclass
	export HOST_CFLAGS=${BUILD_CFLAGS}

	# Export the source path for bindgen
	export CXX_ROOT_PATH="${S}"

	# Some Rust crates may want to depend on C++ build output to determine
	# whether to re-run. Export this directory location so that Rust knows which
	# directory to check C++ output.
	export CXX_OUTDIR="$(cros-workon_get_build_dir)/out/Default"

	# Export the binary provided by the dev-rust/grpcio-compiler package.
	GRPC_RUST_PLUGIN_PATH="$(type -P grpc_rust_plugin)" || die
	export GRPC_RUST_PLUGIN_PATH

	# System API location for proto files
	export CROS_SYSTEM_API_ROOT="${SYSROOT}/usr/include/chromeos"

	local features=(
		chromeos
	)

	cros-rust_src_compile --features="${features[*]}"
}

src_compile() {
	# Compile for target (generates static libs)
	platform_src_compile

	# Build rust portion (finish linking in rust)
	floss_build_rust
}

src_install() {
	platform_src_install

	# shellcheck disable=SC2154 # CARGO_TARGET_DIR is defined in cros-rust.eclass
	dobin "${CARGO_TARGET_DIR}/${CHOST}/release/btmanagerd"
	dobin "${CARGO_TARGET_DIR}/${CHOST}/release/btadapterd"
	dobin "${CARGO_TARGET_DIR}/${CHOST}/release/btclient"

	if use bt_dynlib; then
		dolib.so "${OUT}/lib/libbluetooth.so"
	fi

	dobin "${OUT}/mmc_service"

	# Install seccomp policy file.
	insinto /usr/share/policy
	newins "${FILESDIR}/seccomp/floss-seccomp-${ARCH}.policy" floss-seccomp.policy

	# Install D-Bus config
	insinto /etc/dbus-1/system.d
	doins "${FILESDIR}/dbus/org.chromium.bluetooth.conf"

	# Install upstart rules
	insinto /etc/init/
	doins "${FILESDIR}/upstart/btmanagerd.conf"
	doins "${FILESDIR}/upstart/btadapterd.conf"

	# Install sysprop config file and override dir
	insinto /etc/bluetooth
	doins "${FILESDIR}/sysprops.conf"
	keepdir "/etc/bluetooth/sysprops.conf.d"

	# Change permissions so root can write and bluetooth can read
	chown -R root:bluetooth "${ED}"/etc/bluetooth/sysprops.conf.d
	chmod 750 "${ED}"/etc/bluetooth/sysprops.conf.d

	# Install Android stable flags (aflags)
	insinto /etc/bluetooth/sysprops.conf.d
	doins "${FILESDIR}/config/aflags.conf"

	# Install unstable aflags. This is not installed to sysprops.conf.d and
	# Floss would make decision whether to copy this into sysprops.conf.d.
	insinto /etc/bluetooth
	doins "${FILESDIR}/config/unstable_aflags.conf"

	# Install tmpfiles (don't forget to update sepolicy if you change the
	# files/folders created to something other than /var/lib/bluetooth)
	dotmpfiles "${FILESDIR}/tmpfiles.d/floss.conf"

	# Install config files
	insinto /etc/bluetooth/
	doins "${FILESDIR}/config/bt_did.conf"
	doins "${FILESDIR}/config/bt_stack.conf"
	doins "${FILESDIR}/config/admin_policy.json"
	doins "${FILESDIR}/config/interop_database.conf"

	# Install udev rules
	udev_dorules "${FILESDIR}/udev/99-floss-chown-properties.rules"
	udev_dorules "${FILESDIR}/udev/99-bluetooth-fix-incorrect-input-class.rules"

	# Install chipset-specific bluetooth sysprops.
	insopts -m0640
	insinto "/etc/bluetooth/sysprops.conf.d"
	if use bluetooth_chip_mvl8897; then
		doins "${FILESDIR}/mvl8897_quirk_override.conf"
	fi
	insopts -m0644
}

pkg_preinst() {
	enewuser mmc_service
	enewgroup mmc_service
}

platform_pkg_test() {
	local tests=(
		"hfp_lc3_mmc_encoder_test"
		"hfp_lc3_mmc_decoder_test"
		# TODO(b/190750167) - Re-enable once we're fully Bazel build
		#"bluetoothtbd_test"
		#"bluetooth_test_common"
		#"net_test_avrcp"
		#"net_test_btcore"
		#"net_test_types"
		#"net_test_btm_iso"
		## TODO(b/178740721) - This test wasn't compiling. Need to fix
		## this and re-enable it.
		## "net_test_btpackets"
	)

	# Run rust tests
	# TODO(b/210127355) - Fix flaky tests and re-enable
	# cros-rust_src_test

	local test_bin
	for test_bin in "${tests[@]}"; do
		platform_test run "${OUT}/${test_bin}"
	done
	:
}
