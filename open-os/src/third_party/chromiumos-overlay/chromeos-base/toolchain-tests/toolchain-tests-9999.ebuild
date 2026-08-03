# Copyright 2018 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

PYTHON_COMPAT=( python3_11 )

CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_LOCALNAME="platform2"
CROS_WORKON_DESTDIR="${S}/platform2"
CROS_WORKON_SUBTREE=".gn common-mk cros-toolchain"

# We don't use GTest, so parallel gtest support shouldn't be used.
PLATFORM_PARALLEL_GTEST_TEST=no
PLATFORM_SUBDIR="cros-toolchain"

inherit cros-sanitizers cros-workon platform python-any-r1 cros-toolchain-funcs

DESCRIPTION="Compilation and runtime tests for toolchain"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/third_party/autotest/"

LICENSE="BSD-Google"
KEYWORDS="~*"

BDEPEND="sys-devel/autofdo"

# These dependencies are used on FORTIFY test failures.
DEPEND="
	test? (
		sys-apps/grep
		sys-apps/sed
	)
"

src_unpack() {
	# Only unpack things if we're testing; no point otherwise.
	if ! use test; then
		S="${T}"
		return 0
	fi

	platform_src_unpack
}

src_prepare() {
	# There should never be patches here, but portage gets angry if
	# eapply_user isn't called, so...
	eapply_user
}

src_configure() {
	use test || return 0

	tc-export CXX
	platform_src_configure
}

src_compile() {
	use test || return 0

	platform_src_compile
}

src_install() {
	# Never install anything.
	:
}

# b/389661310: Essentially all AFDO tooling is run on a few toolchain-specific
# builders. There's no testing of it otherwise, which led to a breakage in these
# builders since the binary failed to even load.
ensure_create_llvm_prof_works() {
	local exit_code output
	output="$(create_llvm_prof --help 2>&1)"
	exit_code="$?"

	# Since this was invoked with just `--help`, gflags should output many
	# flags and a prompt to use `--helpfull`, and exit with status 1.
	if [[ "${exit_code}" == 1 && "${output}" == *"--helpfull"* ]]; then
		return 0
	fi

	# Use `echo` instead of `eerror` here so portage won't repeat it
	# unnecessarily.
	echo "Failed to print --help from create_llvm_prof."
	echo "Stdout/stderr was:"
	echo "${output}"
	echo "================="
	echo "Exit code was: ${exit_code}"
	echo "Wanted stdout/stderr to contain '--helpfull', and exit code to be 1."
	return 1
}

src_test() {
	einfo "Testing llvm-profdata..."
	"${FILESDIR}/llvm-profdata-test.py" || die

	einfo "Ensuring AFDO tooling loads..."
	ensure_create_llvm_prof_works || die

	# Skip testing FORTIFY if sanitizers that disable it are enabled; the
	# platform2 build system won't build any FORTIFY artifacts, since
	# ASAN/MSAN/etc disables FORTIFY.
	if use asan || use msan || use ubsan; then
		einfo "Skipping FORTIFY tests on sanitizer builds"
	else
		einfo "Testing FORTIFY diagnostics..."
		./fortify-tests/verify-fortify-diags.sh || die

		einfo "Testing FORTIFY run-time crashes..."
		platform_test run "${OUT}/fortify_tests"
	fi
}
