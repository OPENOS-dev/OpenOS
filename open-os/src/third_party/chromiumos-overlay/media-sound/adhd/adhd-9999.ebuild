# Copyright 2012 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

EAPI=7

PYTHON_COMPAT=( python3_11 )

CROS_WORKON_INCREMENTAL_BUILD=1
CROS_WORKON_PROJECT=(
	"chromiumos/third_party/adhd"
	"chromiumos/third_party/webrtc-apm"
	"chromiumos/platform2"
)
CROS_WORKON_LOCALNAME=(
	"adhd"
	"webrtc-apm"
	"../platform2"
)
CROS_WORKON_SUBTREE=(
	""
	""
	"common-mk"
)
CROS_WORKON_DESTDIR=(
	"${S}/adhd"
	"${S}/webrtc-apm"
	"${S}/platform2"
)
CROS_WORKON_USE_VCSID=1

inherit python-any-r1 cros-toolchain-funcs cros-bazel cros-fuzzer cros-sanitizers cros-workon
inherit cros-debug systemd user cros-protobuf cros-python311-suppress-leak-detection
inherit adhd
# edit this line for adhd.eclass changes.

DESCRIPTION="Google A/V Daemon"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/third_party/adhd/"

LICENSE="Apache-2.0 BSD-Google MIT"
KEYWORDS="~*"

COMMON_DEPEND="
	chromeos-base/chromeos-config-tools:=
	chromeos-base/featured:=
	chromeos-base/libsegmentation:=
	>=chromeos-base/metrics-0.0.1-r3152:=
	chromeos-base/percetto:=
	dev-cpp/abseil-cpp:=
	dev-libs/libevent:=
	dev-libs/openssl:0=
	>=media-libs/alsa-lib-1.1.6-r3:=
	media-libs/sbc:=
	media-libs/speex:=
	>=media-sound/cras_rust-0.1.1:=
	cras-ml? ( sci-libs/tensorflow:= )
	>=sys-apps/dbus-1.4.12:=
	selinux? ( sys-libs/libselinux:= )
	virtual/libudev:=
	test? (
		app-shells/dash
		sys-apps/diffutils
		sys-apps/grep
		sys-apps/which
	)
"

RDEPEND="
	${COMMON_DEPEND}
	acct-user/cras
	acct-group/bluetooth-audio
	acct-group/cras
	chromeos-base/chromeos-config
	media-sound/alsa-utils
	dlc? (
		media-sound/sr-bt-dlc:=
		virtual/chromeos-audio-nc-ap-dlc:=
		virtual/chromeos-audio-speech-enhancer-dlc:=
	)
	media-plugins/alsa-plugins
	!<media-sound/cras_rust-0.1.1
	!media-libs/webrtc-apm
	virtual/udev
"

# dev-cpp/gtest is needed as a transitive dependency of a header included via
# libchrome.
DEPEND="
	${COMMON_DEPEND}
	dev-cpp/gtest
	dev-libs/libpthread-stubs:=
	test? (
		dev-lang/python
		sys-apps/coreutils
	)
"

BDEPEND="
	chromeos-base/minijail
	sys-apps/which
	sys-devel/gettext
	${PYTHON_DEPS}
"

src_configure() {
	cros_optimize_package_for_speed
	adhd_src_configure
}

src_compile() {
	rm -f "${T}/media_sound_adhd.tar" "${T}/fuzzers.tar"
	rm -rf "${T}/fuzzers"

	if ! use fuzzer ; then
		adhd_build_tar media_sound_adhd.tar
	else
		adhd_build_tar fuzzers.tar
		mkdir "${T}/fuzzers"
		tar -C "${T}/fuzzers" -xf "${T}/fuzzers.tar" || die
	fi

	# Add license for vendored code for license scanning.
	mkdir -p "external/iniparser" || die
	cp "bazel-out/../../../external/iniparser/LICENSE" \
		"external/iniparser/LICENSE" || die

	# Add license for thesofproject/sof source code for license scanning.
	# Note: it's named LICEN'C'E in thesofproject_sof whilst LICEN'S'E in iniparser.
	mkdir -p "external/thesofproject_sof" || die
	cp "bazel-out/../../../external/thesofproject_sof/LICENCE" \
		"external/thesofproject_sof/LICENCE" || die
}

src_test() {
	export JAVA_HOME=$(ROOT="${BROOT}" java-config --jdk-home)

	if use fuzzer ; then
		elog "Skipping unit tests on fuzzer build"
		return
	fi

	local platform2_test_py="${S}/platform2/common-mk/platform2_test.py"

	args=(
		"--test_output=errors"
		"--keep_going"

		"--run_under=${FILESDIR}/symbolize_run.sh ${platform2_test_py} --sysroot=${SYSROOT} --strategy=unprivileged --user=root --"

		# Running tests is cheap compared to the build time, don't cache test results.
		"--cache_test_results=no"

		# Pass sanitizer environment variables to the test executable.
		# Also override log_path so errors are shown immediately after
		# the test failure, instead of displayed by asan_death_hook
		# at the bottom of emerge's output:
		# https://source.chromium.org/chromiumos/chromiumos/codesearch/+/main:src/third_party/chromiumos-overlay/profiles/base/profile.bashrc;l=494;drc=14244882a39e40a61fdcdfeec156592bb00f3905
		"--test_env=ASAN_OPTIONS=${ASAN_OPTIONS} log_path=stderr"
		"--test_env=MSAN_OPTIONS=${MSAN_OPTIONS} log_path=stderr"
		"--test_env=TSAN_OPTIONS=${TSAN_OPTIONS} log_path=stderr"
		"--test_env=UBSAN_OPTIONS=${UBSAN_OPTIONS} log_path=stderr"
		"--test_env=LSAN_OPTIONS"
		"--test_env=SYSROOT=${SYSROOT}"

		# profile.bashrc sets LLVM_PROFILE_FILE to tell the path to write *.profraw files.
		"--test_env=LLVM_PROFILE_FILE"

		"--"
		"//..."
	)
	if use cras-apm; then
		args+=(
			"@webrtc_apm//:tests"
		)
	fi
	cd "${S}/adhd" || die

	# Necessary because this test invokes platform2_test.py.
	suppress_python311_leak_detection

	# shellcheck disable=SC2154 # common_bazel_args set by adhd.eclass.
	adhd_bazel test "${common_bazel_args[@]}" "${args[@]}"

	undo_suppress_python311_leak_detection
}

src_install() {
	cd "${S}/adhd" || die

	# Install seccomp policy file.
	insinto /usr/share/policy
	newins "seccomp/cras-seccomp-${ARCH}.policy" cras-seccomp.policy

	# Install asound.conf for CRAS alsa plugin
	insinto /etc
	doins "${FILESDIR}"/asound.conf

	if ! use fuzzer ; then
		einfo Installing media_sound_adhd.tar
		tar -C "${D}" -xf "${T}/media_sound_adhd.tar" || die
	else
		# Install example dsp.ini file for fuzzer
		insinto /etc/cras
		doins cras-config/dsp.ini.sample
		# Install fuzzer binary
		local fuzzer_component_id="890231"
		fuzzer_install "${S}/adhd/OWNERS.fuzz" "${T}/fuzzers"/cras_rclient_message_fuzzer \
			--comp "${fuzzer_component_id}"
		fuzzer_install "${S}/adhd/OWNERS.fuzz" "${T}/fuzzers"/cras_hfp_slc_fuzzer \
			--dict "${S}/adhd/cras/fuzz/cras_hfp_slc.dict" \
			--comp "${fuzzer_component_id}"
		local fuzzer_component_id="769744"
		fuzzer_install "${S}/adhd/OWNERS.fuzz" "${T}/fuzzers"/cras_fl_media_fuzzer \
			--comp "${fuzzer_component_id}"
	fi
}
