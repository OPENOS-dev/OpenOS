# Copyright 2024 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# shellcheck disable=SC2034 # Used by bazel.eclass.
BAZEL_BINARY="/usr/bin/bazel-6"
BDEPEND="=dev-util/bazel-6*"

# The URIs are separated into two parts.
# 1. Explicit dependencies listed in WORKSPACE.bazel.
#    These can be generated with:
#    bazel build //repositories/http_archive_deps:bazel_external_uris
# 2. Implicit dependencies of Bazel. Manitained manually.
bazel_external_uris="
	https://github.com/aspect-build/bazel-lib/releases/download/v2.0.3/bazel-lib-v2.0.3.tar.gz -> aspect-build-bazel-lib-v2.0.3.tar.gz
	https://github.com/bazelbuild/bazel-skylib/releases/download/1.4.1/bazel-skylib-1.4.1.tar.gz -> bazelbuild-bazel-skylib-1.4.1.tar.gz
	https://github.com/bazelbuild/platforms/releases/download/0.0.10/platforms-0.0.10.tar.gz -> bazelbuild-platforms-0.0.10.tar.gz
	https://github.com/bazelbuild/rules_cc/releases/download/0.0.6/rules_cc-0.0.6.tar.gz -> bazelbuild-rules_cc-0.0.6.tar.gz
	https://github.com/bazelbuild/rules_license/releases/download/0.0.7/rules_license-0.0.7.tar.gz -> bazelbuild-rules_license-0.0.7.tar.gz
	https://github.com/bazelbuild/rules_pkg/releases/download/0.10.1/rules_pkg-0.10.1.tar.gz -> bazelbuild-rules_pkg-0.10.1.tar.gz
	https://github.com/bazelbuild/rules_python/releases/download/0.24.0/rules_python-0.24.0.tar.gz -> bazelbuild-rules_python-0.24.0.tar.gz
	https://github.com/bazelbuild/rules_rust/releases/download/0.42.1/rules_rust-v0.42.1.tar.gz -> bazelbuild-rules_rust-v0.42.1.tar.gz
	https://github.com/google/benchmark/archive/refs/tags/v1.7.1.tar.gz -> google-benchmark-v1.7.1.tar.gz
	https://github.com/ndevilla/iniparser/archive/refs/tags/v4.1.tar.gz -> ndevilla-iniparser-v4.1.tar.gz
	https://github.com/thesofproject/sof/archive/refs/tags/v2.5.tar.gz -> thesofproject-sof-v2.5.tar.gz
	https://storage.googleapis.com/chromiumos-test-assets-public/tast/cros/audio/the-quick-brown-fox_20230131.wav
	https://storage.googleapis.com/chromiumos-test-assets-public/tast/cros/audio/the-quick-brown-fox_20240808.zip

	https://github.com/bazelbuild/rules_java/archive/7cf3cefd652008d0a64a419c34c13bdca6c8f178.zip -> bazelbuild-rules_java-7cf3cefd652008d0a64a419c34c13bdca6c8f178.zip
	https://github.com/bazelbuild/rules_java/releases/download/5.5.1/rules_java-5.5.1.tar.gz
	https://mirror.bazel.build/bazel_coverage_output_generator/releases/coverage_output_generator-v2.5.zip
	https://mirror.bazel.build/github.com/bazelbuild/rules_proto/archive/7e4afce6fe62dbff0a4a03450143146f9f2d7488.tar.gz -> bazelbuild-rules_proto-7e4afce6fe62dbff0a4a03450143146f9f2d7488.tar.gz
	https://mirror.bazel.build/openjdk/azul-zulu11.50.19-ca-jdk11.0.12/zulu11.50.19-ca-jdk11.0.12-linux_x64.tar.gz -> bazel-zulu11.50.19-ca-jdk11.0.12-linux_x64.tar.gz
	https://mirror.bazel.build/bazel_java_tools/releases/java/v11.6/java_tools-v11.7.1.zip
	https://mirror.bazel.build/bazel_java_tools/releases/java/v11.6/java_tools_linux-v11.7.1.zip
"

SRC_URI="${bazel_external_uris}"

IUSE="asan +cras-apm cras-debug cras-ml cros-debug dlc featured fuzzer neon selinux systemd test upstream-ucm2"

adhd_get_build_dir() {
	if use coverage; then
		# We don't want incremental builds for coverage:
		# 1. Tests are not re-run between invocations of incremental builds.
		# 2. profile.bashrc does not know to look into cros-workon_get_build_dir.
		# So follow the behavior of non-incremental builds of
		# cros-workon_get_build_dir.
		echo "${WORKDIR}/build"
	else
		cros-workon_get_build_dir
	fi
}

adhd_bazel() {
	bazel_setup_bazelrc
	set -- bazel --bazelrc="${T}/bazelrc" --output_user_root="$(adhd_get_build_dir)" "$@"
	echo "${*}" >&2
	"$@" || die "adhd_bazel failed"
}

use_label() {
	usex "$1" --"$2" --no"$2"
}

adhd_src_unpack() {
	bazel_load_distfiles "${bazel_external_uris}"
	cros-workon_src_unpack

	# Fix up for license scanning.
	# Remove license template file.
	rm "${S}/adhd/LICENSE.tpl" || die
	# devtools/ are not used to build this package.
	rm -r "${S}/adhd/devtools" || die
}

adhd_src_prepare() {
	export JAVA_HOME=$(ROOT="${BROOT}" java-config --jdk-home)
	sanitizers-setup-env
	default
}

adhd_src_configure() {
	export JAVA_HOME=$(ROOT="${BROOT}" java-config --jdk-home)

	python_setup

	cros-debug-add-NDEBUG

	if use amd64 ; then
		export FUZZER_LDFLAGS="-fsanitize=fuzzer"
	fi

	common_bazel_args=(
		# For libcras.pc generation only.
		# cros-bazel.eclass gives /build/<board>/usr, which is not what we want.
		# Restore the bazel.eclass behavior.
		"--define=PREFIX=${EPREFIX%/}/usr"

		"--config=clang-strict"
		"--override_repository=rules_rust=${S}/adhd/cras/rules_rust_stub"
		"--override_repository=com_google_absl=${S}/adhd/third_party/system_absl"
		"--define=VCSID=${VCSID}"
		"--//:chromeos"
		"--//:featured"
		"--//:hats"
		"--//:hw_dependency"
		"--//:metrics"
		"--//:system_percetto"
		"--//dist:libdir=$(get_libdir)"
		"$(use_label cras-apm //:apm)"
		"$(use_label cras-ml //:ml)"
		"$(use_label dlc //:dlc)"

		"--//:system_cras_rust"
		"--repo_env=SYSTEM_CRAS_RUST_LIB=${ESYSROOT}/usr/$(get_libdir)/libcras_rust.a"

		# Allow all warnings to be bypassed for testing LLVM-Next.
		"--action_env=FORCE_DISABLE_WERROR"

		# max-page-size=4096 to work around issues in breakpad's dump_syms
		# See b/308145868
		"--linkopt=-Wl,-z,max-page-size=4096"
	)
	if use cras-apm; then
		common_bazel_args+=(
			"--@webrtc_apm//:chromiumos"
		)
		common_bazel_args+=(
			"$(use_label neon @webrtc_apm//:neon)"
		)
	fi
	if use fuzzer; then
		common_bazel_args+=(
			"--config=fuzzer"
			# Selinux does not work with fuzzers.
			"--no//:selinux"
		)
	else
		common_bazel_args+=(
			"$(use_label selinux //:selinux)"
		)
	fi
	if use ubsan; then
		common_bazel_args+=(
			# Bazel links using C mode by default, which misses symbols.
			# https://github.com/bazelbuild/bazel/issues/11122#issuecomment-896613570
			"--linkopt=-fsanitize-link-c++-runtime"
		)
	fi
	if use coverage; then
		common_bazel_args+=(
			# Embed the absolute path for coverage.
			"--copt=-fcoverage-compilation-dir=${S}/adhd"
		)
	fi
	if [[ "${PV}" != "9999" ]]; then
		common_bazel_args+=(
			# Disable fancy output when not being "workon" to not spam CQ logs.
			"--color=no"
			"--curses=no"
		)
	fi
	if use cras-debug; then
		common_bazel_args+=(
			"--compilation_mode=dbg"
		)
	fi
	if use upstream-ucm2; then
		common_bazel_args+=(
			"--no//:ucm2_files"
			"--//:cras_always_add_controls_and_iodevs_with_ucm"
		)
	fi
}

adhd_build_tar() {
	export JAVA_HOME=$(ROOT="${BROOT}" java-config --jdk-home)

	# Prevent clang to access ubsan_blocklist.txt which is not supported by bazel.
	filter-flags -fsanitize-blacklist="${S}"/ubsan_blocklist.txt
	bazel_setup_crosstool

	cd "${S}/adhd" || die

	adhd_bazel build "${common_bazel_args[@]}" "//dist:$1"
	cp "bazel-bin/dist/$1" "${T}/$1" || die
}

EXPORT_FUNCTIONS src_unpack src_prepare src_configure
