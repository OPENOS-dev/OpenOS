# Copyright 2012 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_WORKON_COMMIT=("906136da2c93e6dfe3c93763c93dfae59d24e54d" "1fb2dc91482efe1956a84e1022a297dd48f721a0" "0e55aeca367c80a0b8570535b4e50e18897eed9d")
CROS_WORKON_TREE=("71e8653b25816e9271e8b6751ddb0ff9ec131555" "518b50f8b6d01e95cbd933487ed7c6452ac4acb3" "a737255cd6e3b6dbc4084583500ad07005237451")
PYTHON_COMPAT=( python3_11 )

# platform2 is used purely for the platform2_test.py wrapper

CROS_WORKON_PROJECT=(
	"chromiumos/platform/vboot_reference"
	"chromiumos/platform2"
	"platform/external/avb"
)
CROS_WORKON_LOCALNAME=(
	"platform/vboot_reference"
	"platform2"
	"aosp/external/avb"
)
CROS_WORKON_SUBTREE=("" "common-mk" "libavb")
LIBAVB_DESTDIR="${S}/libavb"
CROS_WORKON_DESTDIR=("${S}/vboot_reference" "${S}/platform2" "${LIBAVB_DESTDIR}")

inherit cros-debug cros-fuzzer cros-sanitizers cros-subtool cros-workon cros-constants python-any-r1

DESCRIPTION="Chrome OS verified boot tools"
LICENSE="BSD-Google"
KEYWORDS="*"
IUSE="avb cros_host dev_debug_force fuzzer pd_sync test tpmtests tpm tpm_dynamic tpm2 tpm2_simulator +flashrom"

REQUIRED_USE="
	tpm_dynamic? ( tpm tpm2 )
	!tpm_dynamic? ( ?? ( tpm tpm2 ) )
"

COMMON_DEPEND="
	!cros_host? ( chromeos-base/crosid:= )
	app-arch/libarchive:=
	dev-libs/libzip:=
	dev-libs/nss:=
	dev-libs/openssl:=
	dev-util/shflags:=
	flashrom? ( sys-apps/flashrom:= )
	sys-apps/util-linux:="

# Some tests are shell scripts, which require these utilities.
TEST_DEPEND="
	${PYTHON_DEPS}
	app-misc/jq
	app-shells/bash
	dev-libs/openssl
	dev-util/xxd
	sys-apps/coreboot-utils
	sys-apps/coreutils
	sys-apps/diffutils
	sys-apps/grep
	sys-apps/sed
	sys-devel/bc
	app-alternatives/awk
"

# futility needs coreboot-utils's cbfstool in run time. See CL:3575325.
RDEPEND="
	${COMMON_DEPEND}
	sys-apps/coreboot-utils:=
"
BDEPEND="
	virtual/pkgconfig
	test? (
		${TEST_DEPEND}
	)
"
# PDEPEND is for runtime dependencies that can be merged after the package,
# to solve the circular dependency by platform.eclass.
# futility needs ec-util's ectool in run time. See CL:5804408.
PDEPEND="
	chromeos-base/ec-utils
"

# platform2_test.py runs the tests inside sysroot, so DEPEND needs to include
# test dependencies as well.
DEPEND="
	${COMMON_DEPEND}
	test? (
		${TEST_DEPEND}
	)
"

get_build_dir() {
	echo "${S}/build-main"
}

src_configure() {
	# Determine sanitizer flags. This is necessary because the Makefile
	# purposely ignores CFLAGS from the environment. So we collect the
	# sanitizer flags and pass just them to the Makefile explicitly.
	SANITIZER_CFLAGS=
	append-flags() {
		SANITIZER_CFLAGS+=" $*"
	}
	sanitizers-setup-env
	if use_sanitizers; then
		# Disable alignment sanitization, https://crbug.com/1015908 .
		SANITIZER_CFLAGS+=" -fno-sanitize=alignment"

		# Run sanitizers with useful log output.
		SANITIZER_CFLAGS+=" -DVBOOT_DEBUG"

		# Suppressions for unit tests.
		if use test; then
			# Do not check memory leaks or odr violations in address sanitizer.
			# https://crbug.com/1015908 .
			export ASAN_OPTIONS+=":detect_leaks=0:detect_odr_violation=0:"
			# Suppress array bound checks, https://crbug.com/1082636 .
			SANITIZER_CFLAGS+=" -fno-sanitize=array-bounds"
		fi
	fi
	cros-debug-add-NDEBUG
	default
}

vemake() {
	emake -C "${S}/vboot_reference" \
		SRCDIR="${S}/vboot_reference" \
		LIBDIR="$(get_libdir)" \
		ARCH="$(tc-arch)" \
		SDK_BUILD=$(usev cros_host) \
		TPM2_MODE=$(usev tpm2) \
		PD_SYNC=$(usev pd_sync) \
		DEV_DEBUG_FORCE=$(usev dev_debug_force) \
		EXTERNAL_TPM_CLEAR_REQUEST="$(usev tpm_dynamic)$(usev tpm2_simulator)" \
		FUZZ_FLAGS="${SANITIZER_CFLAGS}" \
		BUILD="$(get_build_dir)" \
		USE_FLASHROM="$(usev flashrom)" \
		HAVE_CROSID="$(usex cros_host 0 1)" \
		USE_AVB="$(usex test 1 $(usex avb 1 0))" \
		LIBAVB_SRCDIR="${LIBAVB_DESTDIR}" \
		"$@"
}

src_compile() {
	mkdir "$(get_build_dir)"
	tc-export CC AR CXX PKG_CONFIG
	# vboot_reference knows the flags to use
	unset CFLAGS
	vemake all $(usex fuzzer fuzzers '')
}

src_test() {
	# Supply a test wrapper, platform2_test.py. Run the pre-test setup (binfmt)
	# beforehand so that multiple tests don't race each other trying to do it.
	#
	# vboot_reference test scripts use 'BUILD_RUN' for the build dir inside the
	# wrapper. platform2_test would filter that out of the environment so we
	# ensure that it gets through using --env.
	local p2t="${S}/platform2/common-mk/platform2_test.py"
	local p2t_flags="$(usex cros_host --host --sysroot="${SYSROOT}")"
	p2t_flags+=" --env=BUILD_RUN=\${BUILD_RUN}"
	# Allow VBOOT_TEST_USE_FLASHROM (set by Makefile based on USE_FLASHROM)
	# to pass through the p2t wrapper, controlling flashrom-dependent tests.
	p2t_flags+=" --env=VBOOT_TEST_USE_FLASHROM=\${VBOOT_TEST_USE_FLASHROM}"

	# We can't really quote arguments here in a way that would also work
	# correctly for passing the variable to `make` below. Thankfully, our
	# sysroot paths never have spaces.
	# shellcheck disable=SC2086
	"${p2t}" ${p2t_flags} --action=pre_test
	vemake \
		RUNTEST="${p2t} --action=run ${p2t_flags} --" \
		runtests

	# Make sure we don't break the USE_FLASHROM=0 build.
	einfo "Running tests with USE_FLASHROM=0"
	vemake \
		BUILD="${S}/build-no-flashrom" \
		USE_FLASHROM=0 \
		RUNTEST="${p2t} --action=run ${p2t_flags} --" \
		futil runtests
}

src_install() {
	einfo "Installing programs"
	vemake \
		DESTDIR="${D}" \
		install install_dev

	if use cros_host; then
		cros-subtool_src_install "${S}"/vboot_reference/*/*_subtool.textproto
	fi

	if use tpmtests; then
		into /usr
		# copy files starting with tpmtest, but skip .d files.
		dobin "$(get_build_dir)"/tests/tpm_lite/tpmtest*[^.]?
		dobin "$(get_build_dir)"/utility/tpm_set_readsrkpub
	fi

	if use fuzzer; then
		einfo "Installing fuzzers"
		local fuzzer_component_id="167186"
		fuzzer_install "${S}"/vboot_reference/OWNERS "$(get_build_dir)"/tests/cgpt_fuzzer \
			--comp "${fuzzer_component_id}"
		fuzzer_install "${S}"/vboot_reference/OWNERS "$(get_build_dir)"/tests/vb2_keyblock_fuzzer \
			--comp "${fuzzer_component_id}"
		fuzzer_install "${S}"/vboot_reference/OWNERS "$(get_build_dir)"/tests/vb2_preamble_fuzzer \
			--comp "${fuzzer_component_id}"
	fi
}
