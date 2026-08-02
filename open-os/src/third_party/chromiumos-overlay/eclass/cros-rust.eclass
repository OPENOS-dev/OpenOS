# Copyright 2018 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

# NOTE: If you make changes to this file that require Rust code to
# be rebuilt, you can change the revision on virtual/rust-binaries
# to make that rebuild happen on the next build_packages run.

# @ECLASS: cros-rust.eclass
# @MAINTAINER:
# The ChromiumOS Authors <chromium-os-dev@chromium.org>
# @BUGREPORTS:
# Please report bugs via https://crbug.com/new (with component "Tools>ChromeOS-Toolchain")
# @VCSURL: https://chromium.googlesource.com/chromiumos/overlays/chromiumos-overlay/+/HEAD/eclass/@ECLASS@
# @BLURB: Eclass for fetching, building, and installing Rust packages.

if [[ -z ${_ECLASS_CROS_RUST} ]]; then
_ECLASS_CROS_RUST="1"

# Check for EAPI 7+.
case "${EAPI:-0}" in
[0123456]) die "unsupported EAPI (${EAPI}) in eclass (${ECLASS})" ;;
esac

# @ECLASS-VARIABLE: CROS_RUST_CRATE_NAME
# @DESCRIPTION:
# The name of the crate used by Cargo. This defaults to the package name.
: "${CROS_RUST_CRATE_NAME:=${PN}}"

# @ECLASS-VARIABLE: CROS_RUST_CRATE_VERSION
# @DESCRIPTION:
# The version of the crate used by Cargo. This defaults to PV. Note that
# cros-rust_get_crate_version can be used to get this information from the
# Cargo.toml but that is only available in src_* functions. Also, for -9999
# ebuilds this is handled in a special way; A symbolic link is used to point to
# the installed crate so it can be removed correctly.
: "${CROS_RUST_CRATE_VERSION:=${PV}}"

# @ECLASS-VARIABLE: CROS_RUST_OVERFLOW_CHECKS
# @PRE_INHERIT
# @DESCRIPTION:
# Enable integer overflow checks for this package.  Packages that wish to
# disable integer overflow checks should set this value to 0.  Integer overflow
# checks are always enabled when the cros-debug flag is set.
: "${CROS_RUST_OVERFLOW_CHECKS:=1}"

# @ECLASS-VARIABLE: CROS_RUST_REMOVE_DEV_DEPS
# @PRE_INHERIT
# @DESCRIPTION:
# Removes all the dev-dependencies from the Cargo.toml. This can break circular
# dependencies and help minimize how many dependent packages need to be added.
: "${CROS_RUST_REMOVE_DEV_DEPS:=}"

# @ECLASS-VARIABLE: CROS_RUST_REMOVE_TARGET_CFG
# @PRE_INHERIT
# @DESCRIPTION:
# Removes all the target. sections from the Cargo.toml except cfg(unix),
# cfg(linux), cfg(not(windows), and *-linux-gnu. Note that this does not handle
# more complicated cfg strings, so those cases should be handled manually
# instead of using this option.
: "${CROS_RUST_REMOVE_TARGET_CFG:=}"

# @ECLASS-VARIABLE: CROS_RUST_SUBDIR
# @DESCRIPTION:
# Subdir where the package is located. Only used by cros-workon ebuilds.
: "${CROS_RUST_SUBDIR:=${CROS_RUST_CRATE_NAME}}"

# @ECLASS-VARIABLE: CROS_RUST_TESTS
# @DESCRIPTION:
# An array of test executables to be run, which defaults to empty value and is
# set by invoking cros-rust_get_test_executables.
: "${CROS_RUST_TESTS:=}"

# @ECLASS-VARIABLE: CROS_RUST_HOST_TESTS
# @DESCRIPTION:
# An array of test executables that are built for cros-host, which defaults to
# empty value and is set by invoking cros-rust_get_host_test_executables.
# If it is empty when cros-rust_get_test_executables is called, it will be set
# to include tests not compiled for ${CHOST}.
: "${CROS_RUST_HOST_TESTS:=}"

# @ECLASS-VARIABLE: CROS_RUST_PLATFORM_TEST_ARGS
# @DESCRIPTION:
# An array of arguments to pass to platform2_test.py such as --no-ns-net,
# --no-ns-pid, or --run_as_root.

# @ECLASS-VARIABLE: CROS_RUST_TEST_DIRECT_EXEC_ONLY
# @DESCRIPTION:
# If set to yes, run the test only for amd64 and x86 (i.e. no emulation).
: "${CROS_RUST_TEST_DIRECT_EXEC_ONLY:="no"}"

# @ECLASS-VARIABLE: CROS_RUST_TEST_MULTIPROCESS
# @PRE_INHERIT
# @DESCRIPTION:
# If set to yes, run test binaries in parallel but without affecting
# `--test-threads`` on individual test binaries.
# TODO(b/293327433): Temporarily disabled due to flaky tests
: "${CROS_RUST_TEST_MULTIPROCESS:="no"}"

# @ECLASS-VARIABLE: CROS_RUST_PACKAGE_IS_HOT
# @DESCRIPTION:
# If set to a nonempty value, we will consider the binaries we compile to be
# hot, and optimize them more aggressively for speed. Please use the
# `cros_optimize_package_for_speed` function to set this, as that also applies
# the same settings for C and C++ code.
: "${CROS_RUST_PACKAGE_IS_HOT:=}"

# @ECLASS-VARIABLE: CROS_RUST_PREINSTALLED_REGISTRY_CRATE
# @DESCRIPTION:
# If set to a nonempty value, `cros-rust_src_unpack` will also copy sources from
# `${CROS_RUST_REGISTRY_DIR}` into `${S}`, and suppress any automatic publishing
# of Rust sources.
#
# TODO(gbiv): This should ideally `ln` from the registry, rather than `cp`.
# There's quite a bit that wants to write to the crate root though, and the
# registry should be immutable, so a cleanup is needed.
: "${CROS_RUST_PREINSTALLED_REGISTRY_CRATE:=}"

# @ECLASS-VARIABLE: CROS_RUST_NO_COVERAGE
# @DESCRIPTION:
# If set to a nonempty value, code coverage flags will be disabled regardless of
# the value of the `rust-coverage` setting. This is generally only useful if
# you're using a nonstandard Rust toolchain.
: "${CROS_RUST_COVERAGE_DISABLED:=}"

# @ECLASS-VARIABLE: CROS_RUST_INSTALL_SOURCES_ONLY
# @PRE_INHERIT
# @DESCRIPTION:
# If set to a nonempty value, cros-rust assumes that the package in question
# only exists to install source files. Setting this makes it an error to use
# `ecargo` in most ebuild functions. As a convenience, usage of `ecargo` is
# allowed in `src_test` of `FEATURES=test` builds. This allows packages to
# easily self-check using Cargo.
: "${CROS_RUST_INSTALL_SOURCES_ONLY:=}"

# @ECLASS-VARIABLE: CROS_RUST_FORCE_STATIC_LINK
# @DESCRIPTION:
# If set to a nonempty value, libstd will be forcibly statically linked. This
# harms code size, so is not recommended if your ebuild builds a binary that
# lands in ChromeOS' userland.
: "${CROS_RUST_FORCE_STATIC_LINK:=}"

# @ECLASS-VARIABLE: CROS_RUST_DISABLE_EDITION_CHECKS
# @PRE_INHERIT
# @DESCRIPTION:
# If set to a nonempty value, edition checks won't be enabled on this package.
# Our edition checks help keep CrOS on the newest Rust editions, which often
# enable cleaner code.
: "${CROS_RUST_DISABLE_EDITION_CHECKS:=}"

# @ECLASS-VARIABLE: CROS_RUST_EXTRA_ALLOWED_FEATURES
# @DESCRIPTION:
# Array of extra allowed Rust unstable features used during build. Please get
# approval from the toolchain team before setting it, as unstable features
# might be easily broken by toolchain updates.
if [[ ! -v CROS_RUST_EXTRA_ALLOWED_FEATURES ]]; then
	CROS_RUST_EXTRA_ALLOWED_FEATURES=()
fi

inherit multiprocessing cros-toolchain-funcs cros-constants cros-debug cros-sanitizers

IUSE="asan rust-coverage cros_host fuzzer lsan +lto msan +panic-abort test tsan ubsan"
REQUIRED_USE="?? ( asan lsan msan tsan )"

EXPORT_FUNCTIONS pkg_setup src_unpack src_prepare src_configure src_compile src_test src_install pkg_preinst pkg_postinst pkg_prerm

# virtual/rust-binaries is listed in both DEPEND and RDEPEND. Changing the
# version of virtual/rust-binaries forces a rebuild of everything that
# depends on it (that is, all compiled Rust code in ChromeOS).
DEPEND="
	virtual/rust:1=
	virtual/rust-binaries:=
"

RDEPEND="
	virtual/rust-binaries:=
"

# Pull in dynamically linked libraries if we're going to use `-Cprefer-dynamic`.
# `-Cprefer-dynamic` is disabled for fuzzing and sanitizers since its usability
# is unclear with those, and it's disabled for the host since we care less about
# binary size there than on devices.
if [[ -z "${CROS_RUST_PACKAGE_IS_HOT}" && -z "${CROS_RUST_FORCE_STATIC_LINK}" ]]; then
	RDEPEND+="
		!cros_host? (
			!test? ( !asan? ( !lsan? ( !msan? ( !tsan? ( !fuzzer? ( dev-lang/rust-dylibs:= ) ) ) ) ) )
		)
	"
fi

BDEPEND="
	!cros_host? (
		arm? ( cross-armv7a-cros-linux-gnueabihf/rust:= )
		arm64? ( cross-aarch64-cros-linux-gnu/rust:= )
		amd64? ( cross-x86_64-cros-linux-gnu/rust:= )
	)
	dev-lang/rust-host:=
"

_cros-rust_enable_edition_checks() {
	[[ "${PV}" == 9999 && -z "${CROS_RUST_DISABLE_EDITION_CHECKS}" ]]
}

_cros-rust_enable_edition_checks && BDEPEND+="
	dev-util/rust-edition-checker
"

# If we're just installing sources, no need to force dependencies on Rust
# (unless we're testing, as outlined in docs for
# CROS_RUST_INSTALL_SOURCES_ONLY).
if [[ -n "${CROS_RUST_INSTALL_SOURCES_ONLY}" ]]; then
	DEPEND="test? ( ${DEPEND} )"
	RDEPEND="test? ( ${RDEPEND} )"
	BDEPEND="test? ( ${BDEPEND} )"
fi

CROS_RUST_REGISTRY_BASE="/usr/lib/cros_rust_registry"
ECARGO_HOME="${WORKDIR}/cargo_home"
CROS_RUST_REGISTRY_DIR="${CROS_RUST_REGISTRY_BASE}/store"
CROS_RUST_REGISTRY_INST_DIR="${CROS_RUST_REGISTRY_BASE}/registry"
# Crate owners directory. This has one file per crate in
# CROS_RUST_REGISTRY_INST_DIR that describes the package which installed the
# crate's link in CROS_RUST_REGISTRY_INST_DIR. This is needed to support our
# current preinst/postinst/prerm functions without introducing race conditions:
# - prerm will delete a symlink if the symlink is owned by the current package
# - preinst will delete a symlink regardless of ownership
# - postinst installs a new symlink and declares ownership of it
CROS_RUST_REGISTRY_OWNER_DIR="${CROS_RUST_REGISTRY_BASE}/owners"

# The workdir-based config.toml file that e.g., per-arch flags/configuration
# land in.
_CROS_RUST_CONFIG_TOML="${ECARGO_HOME}/config.toml"

# Ignore odr violations in unit tests in asan builds
# (https://github.com/rust-lang/rust/issues/41807).
export ASAN_OPTIONS="detect_odr_violation=0"

_cros-rust_acquire_fd_lock() {
	local fd="$1"
	local extra_args=( "${@:2}" )

	flock --timeout=15 --conflict-exit-code=200 "${extra_args[@]}" "${fd}"
	local returncode="$?"
	case "${returncode}" in
		0 )
			return
			;;
		200 )
			einfo "Acquiring the registry lock is taking a while."
			einfo "If this command hangs indefinitely, you might have old processes hanging onto the lock."
			flock "${extra_args[@]}" "${fd}" || die
			;;

		* )
			die "Unexpected flock return code: ${returncode}"
			;;
	esac
}

# @FUNCTION: cros-rust_get_reg_lock
# @DESCRIPTION:
# Return the path to our rust registry lock file used to prevent races.
cros-rust_get_reg_lock() {
	echo "${ROOT}${CROS_RUST_REGISTRY_BASE}/lock"
}

# @FUNCTION: cros-rust_pkg_setup
# @DESCRIPTION:
# Sets up the package. Particularly, makes sure the rust registry lock exits.
cros-rust_pkg_setup() {
	debug-print-function "${FUNCNAME[0]}" "$@"
	# This triggers a linter error SC2154 which says:
	#   "EBUILD_PHASE_FUNC is used but not defined inside this file"
	# Since EBUILD_PHASE_FUNC comes from outside the file, that's ok
	# shellcheck disable=SC2154
	if [[ "${EBUILD_PHASE_FUNC}" != "pkg_setup" ]]; then
		die "${FUNCNAME[0]}() should only be used in pkg_setup() phase"
	fi
	addwrite "$(cros-rust_get_reg_lock)"
	_cros-rust_prepare_lock

	# This is needed for CROS_WORKON_INCREMENTAL_BUILD to be honored.
	if [[ -n "${CROS_WORKON_PROJECT}" ]]; then
		cros-workon_pkg_setup
	fi
}

# @FUNCTION: cros-rust_src_unpack
# @DESCRIPTION:
# Unpacks the package
cros-rust_src_unpack() {
	debug-print-function "${FUNCNAME[0]}" "$@"

	# If this is a cros-workon ebuild and hasn't been unpacked, then unpack it.
	if [[ -n "${CROS_WORKON_PROJECT}" && ! -e "${S}" ]]; then
		cros-workon_src_unpack
		S+="/${CROS_RUST_SUBDIR}"
	fi

	if [[ -n "${CROS_RUST_PREINSTALLED_REGISTRY_CRATE}" ]]; then
		local registry_dir="${ROOT}${CROS_RUST_REGISTRY_DIR}/${CROS_RUST_CRATE_NAME}-${CROS_RUST_CRATE_VERSION}"
		[[ -d "${registry_dir}" ]] || die "Registry directory ${registry_dir} doesn't exist."
		cp -r "${registry_dir}" "${S}" || die
	fi

	local archive
	for archive in ${A}; do
		case "${archive}" in
			*.crate)
				ebegin "Unpacking ${archive}"

				ln -s "${DISTDIR}/${archive}" "${archive}.tar"
				unpack "./${archive}.tar"
				rm "${archive}.tar"

				eend $?
				;;
			*)
				unpack "${archive}"
				;;
		esac
	done

	if [[ -z "${LICENSE}" ]]; then
		die "Missing LICENSE= setting in ebuild"
	fi
	if [[ "${LICENSE}" == "metapackage" ]]; then
		die "LICENSE=metapackage is not allowed in crate ebuilds"
	fi

	# Set up the cargo config.
	mkdir -p "${ECARGO_HOME}"

	cat > "${_CROS_RUST_CONFIG_TOML}" <<- EOF || die
	[source.chromeos]
	directory = "${SYSROOT}${CROS_RUST_REGISTRY_INST_DIR}"

	[source.crates-io]
	replace-with = "chromeos"
	local-registry = "/nonexistent"

	[build]
	jobs = $(makeopts_jobs)
	EOF

	# When the target environment is different from the host environment,
	# add a setting for the target environment.
	if tc-is-cross-compiler; then
		cat <<- EOF >> "${_CROS_RUST_CONFIG_TOML}" || die

		[target.${CBUILD}]
		linker = "$(tc-getBUILD_CC)"
		EOF
	fi

	# Tell cargo not to use terminal colors if NOCOLOR is set.
	# Shellcheck thinks NOCOLOR is never defined.
	# shellcheck disable=SC2154
	if [[ "${NOCOLOR}" == true || "${NOCOLOR}" == yes ]]; then
		cat <<- EOF >> "${_CROS_RUST_CONFIG_TOML}" || die

		[term]
		color = "never"
		EOF
	fi
}

# @FUNCTION: cros-rust-patch-cargo-toml
# @USAGE: <path to Cargo.toml file>
# @DESCRIPTION:
# Patches the Cargo.toml at "${1}". This function supports
# "# provided by ebuild" macro and "# ignored by ebuild" macro for replacing
# and removing path dependencies.
#
# NOTE: the Cargo.toml will be modified in place. This is not compatible with
# CROS_WORKON_OUTOFTREE_BUILD.
cros-rust-patch-cargo-toml() {
	local cargo_toml_path="${1}"
	[[ -e "${cargo_toml_path}" ]] || die "Provided path doesn't exist"

	# shellcheck disable=SC2154
	if [[ "${CROS_WORKON_OUTOFTREE_BUILD}" == 1 ]]; then
		die "CROS_WORKON_OUTOFTREE_BUILD=1 must not be set when using" \
			"\`provided by ebuild\`"
	fi

	# '# provided by ebuild'
	# Replace path dependencies with ones provided by their ebuild.
	#
	# For local developer builds, we want Cargo.toml to contain path
	# dependencies on sibling crates within the same repository or elsewhere
	# in the Chrome OS source tree. This enables developers to run `cargo
	# build` and have dependencies resolve correctly to their locally
	# checked out code.
	#
	# At the same time, some crates contained within the crosvm repository
	# have their own ebuild independent of the crosvm ebuild so that they
	# are usable from outside of crosvm. Ebuilds of downstream crates won't
	# be able to depend on these crates by path dependency because that
	# violates the build sandbox. We perform a sed replacement to eliminate
	# the path dependency during ebuild of the downstream crates.
	#
	# The sed command says: in any line containing `# provided by ebuild`,
	# please replace `path = "..."` with `version = "*"`. The intended usage
	# is like this:
	#
	#     [dependencies]
	#     data_model = { path = "../data_model" }  # provided by ebuild
	#
	# This also works with `git` attributes:
	#     [dependencies]
	#     bar = { git = "https://www.foo.com", branch = "a" }  # provided by ebuild
	#     foo = { git = "https://www.foo.com", rev = "1234567" }  # provided by ebuild
	#     foo = { git = "https://www.foo.com" }  # provided by ebuild
	#
	# '# ignored by ebuild'
	# Emerge ignores "out-of-sandbox" [patch.crates-io] lines in Cargo.toml.
	sed -i \
		-e '/# ignored by ebuild/d' \
		-e '/# provided by ebuild$/ {
			s/\(path\|git\) = "[^"]*"/version = "*"/
			s/,\? *\(branch\|rev\) = "[^"]*"//
		}' \
		"${cargo_toml_path}" || die
}

# @FUNCTION: cros-rust_src_prepare
# @DESCRIPTION:
# Prepares the src. This function supports "# provided by ebuild" macro and
# "# ignored by ebuild" macro for replacing and removing path dependencies
# with ones provided by their ebuild in Cargo.toml
# and Cargo.toml will be modified in place. If the macro is used in
# ${S}/Cargo.toml, CROS_WORKON_OUTOFTREE_BUILD can't be set to 1 in its ebuild.
cros-rust_src_prepare() {
	debug-print-function "${FUNCNAME[0]}" "$@"
	if grep -q "# provided by ebuild\|# ignored by ebuild" "${S}/Cargo.toml"; then
		cros-rust-patch-cargo-toml "${S}/Cargo.toml"
	fi

	# Remove dev-dependencies and target.cfg sections within the Cargo.toml file
	#
	# The awk program reads the file line by line. If any line matches one of the
	# matched section headers, it will skip every line a new section header is
	# found that does not match one of the matched section headers.
	#
	# Awk cannot do in-place editing, so we write the result to a temporary
	# file before replacing the input with that temp file.
	if [[ "${CROS_RUST_REMOVE_DEV_DEPS}" == 1 ]] || [[ "${CROS_RUST_REMOVE_TARGET_CFG}" == 1 ]]; then
		awk -v rm_dev_dep="${CROS_RUST_REMOVE_DEV_DEPS}" \
		-v rm_target_cfg="${CROS_RUST_REMOVE_TARGET_CFG}" \
		'{
			# Stop skipping for a new section header, but check for another match.
			if ($0 ~ /^\[/) {
				skip = 0
			}

			# If rm_dev_dep is set, match section headers of the following forms:
			#   [token.dev-dependencies]
			#   [dev-dependencies.token]
			#   [dev-dependencies]
			if (rm_dev_dep && ($0 ~ /^\[([^][]+\.)?dev-dependencies(\.[^][]+)?\]$/)) {
				skip = 1
				next
			}

			# If rm_target_cfg is set, match section headers prefixed by `[target.`,
			# but exclude matches that contain any of `cfg(unix`, `cfg(linux`,
			# `cfg(not(windows)`, or `-linux-gnu`.
			if (rm_target_cfg && ($0 ~ /^\[target[.]/) && ($0 !~ /cfg[(](unix|linux|not[(]windows[)])|-linux-gnu/)) {
				skip = 1
				next
			}

			if (skip == 0) {
				print
			}
		}' "${S}/Cargo.toml" > "${S}/Cargo.toml.stripped" || die
		mv "${S}/Cargo.toml.stripped" "${S}/Cargo.toml"|| die
	fi

	default
}

_cros-rust_find_default_libstd_rlib() {
	local libstds
	read -r -a libstds < <(shopt -s nullglob; echo "/usr/lib/rustlib/${CHOST}/lib"/libstd-*.rlib) || die "no libstd.so found"
	[[ "${#libstds[@]}" -ne 1 ]] && die "Expected exactly one libstd; found ${libstds[*]}"
	echo "${libstds[0]}"
}

# @FUNCTION: cros-rust_configure_cargo
# @DESCRIPTION:
# Sets up cargo configuration and exports any environment variables needed
# during the build.
cros-rust_configure_cargo() {
	debug-print-function "${FUNCNAME[0]}"
	sanitizers-setup-env
	cros-debug-add-NDEBUG

	if [[ -n "${CROS_WORKON_PROJECT}" ]]; then
		# Use a sub directory to avoid unintended interactions with platform.eclass.
		export CARGO_TARGET_DIR="$(cros-workon_get_build_dir)/cros-rust"
		mkdir -p "${CARGO_TARGET_DIR}"
	else
		export CARGO_TARGET_DIR="${WORKDIR}"
	fi
	export CARGO_HOME="${ECARGO_HOME}"
	export HOST="${CBUILD}"
	export HOST_CC="$(tc-getBUILD_CC)"
	# PKG_CONFIG_ALLOW_CROSS is required by pkg-config.
	# https://github.com/rust-lang/pkg-config-rs/issues/41.
	# Since cargo will overwrites $HOST with "" when building pkg-config, we
	# need to set it regardless of the value of tc-is-cross-compiler here.
	export PKG_CONFIG_ALLOW_CROSS=1
	export PKG_CONFIG="$(tc-getPKG_CONFIG)"
	export TARGET="${CHOST}"
	export TARGET_CC="$(tc-getCC)"

	# bindgen bypasses the compiler wrapper by using libclang, so it will not use the
	# wrapper configured sysroot, so set it here to ensure all crates pick it up
	if tc-is-cross-compiler; then
		export BINDGEN_EXTRA_CLANG_ARGS="--sysroot=${ESYSROOT}"
	fi

	# Intended use case:
	# - Crate A generates sources when it is emerged from input files
	#   that are only accessible when it emerges.
	# - Crate B depends on crate A, and this is reflected in the
	#   ebuild for crate B.
	# (Examples: cros-dbus-bindings or bindgen for *-sys)
	#
	# The following scenarios are supported and need to work:
	# - local `cargo build` for crate A
	# - local `cargo build` for crate B
	# - emerge A
	# - emerge B
	#
	# Add CROS_RUST environment variable to support the `emerge B`
	# case, since crate B can't access pre-generated source
	# in emerge, the build.rs script for crate A will skip the
	# source generation if both of the following are true:
	# - The generated source exists
	# - `CROS_RUST=1`
	export CROS_RUST="1"

	# There is a memory leak in libbacktrace:
	# https://github.com/rust-lang/rust/issues/59125
	cros-rust_use_sanitizers || export RUST_BACKTRACE=1

	local unstable_features="sanitizer"
	[[ "${#CROS_RUST_EXTRA_ALLOWED_FEATURES[@]}" -ne 0 ]] && \
		unstable_features+=$(printf ",%s" "${CROS_RUST_EXTRA_ALLOWED_FEATURES[@]}")
	# We want to split the flags since it's a command line as a scalar.
	# shellcheck disable=SC2206
	local rustflags=(
		${CROS_BASE_RUSTFLAGS}
		# We want debug info even in release builds.
		"-Cdebuginfo=2"
		"-Cstrip=none"
		# Please get approval from the toolchain team before setting
		# CROS_RUST_EXTRA_ALLOWED_FEATURES in your package, as unstable features
		# might be easily broken by toolchain updates.
		"-Zallow-features=${unstable_features}"
	)

	# b/404353398: Some boards specify flags that introduce instructions
	# that not all builders can execute. Until that's addressed, downgrade
	# target-cpu if we're going to be running unittests.
	if use test && [[ "${ARCH}" == amd64 ]]; then
		rustflags+=( "-Ctarget-cpu=x86-64-v2" )
	fi

	# RUSTFLAGS that should only apply to binaries compiled for '${CHOST}'.
	# The 'rustflags' variable applies to all binaries, incl. '${CBUILD}'
	# and others (e.g., baremetal)
	local rustflags_chost=()

	local skip_dyn_link=false opt_level=s
	# Treat all host binaries as hot enough to opt for speed. The difference
	# in size is minuscule compared to the rest of the SDK, and we don't
	# have hotness data to guide which to optimize.
	if [[ -n "${CROS_RUST_PACKAGE_IS_HOT}" ]] || use cros_host; then
		skip_dyn_link=true
		opt_level=3
	elif use test || use asan || use msan || use lsan || use tsan || use fuzzer; then
		skip_dyn_link=true
	elif ! use panic-abort; then
		ewarn "USE=panic-abort is recommended to save binary size"
		skip_dyn_link=true
	elif [[ -n "${CROS_RUST_FORCE_STATIC_LINK}" ]]; then
		skip_dyn_link=true
	fi

	rustflags+=( "-Copt-level=${opt_level}" )
	if "${skip_dyn_link}"; then
		use lto && rustflags+=(
			"-Clto=thin"
			"-Cllvm-args=--import-instr-limit=30"
			# Cargo sets -Cembed-bitcode to no because it does not
			# know that we want to use LTO. Because -Clto requires
			# -Cembed-bitcode=yes, set it explicitly.
			"-Cembed-bitcode=yes"
		)

		# On targets, libstd.so from the panic=abort sysroot will be
		# visible even when panic=unwind. rustc errors out over this
		# ambiguity; explicitly specify the panic=unwind one.
		if ! use cros_host; then
			local rlib_path="$(_cros-rust_find_default_libstd_rlib)"
			local rmeta_path="${rlib_path%.rlib}.rmeta"
			rustflags_chost+=(
				"--extern"
				"std=${rlib_path}"
			)
			# In  Rust 1.94, `std` was split into `rmeta` and `rlib`.
			# b/522255100: remove the `-e` check once 1.94 has landed.
			if [[ -e "${rmeta_path}" ]]; then
				rustflags_chost+=(
					"--extern"
					"std=${rmeta_path}"
				)
			fi
		fi
	else
		rustflags+=(
			"-Cprefer-dynamic"
			"-Clto=no"
			"--sysroot=/usr/lib/rust-sysroots/panic-abort"
		)
	fi

	# Set the panic=abort flag if it is turned on for the package.
	if use panic-abort; then
		# But never abort during tests.
		use test || rustflags+=( -Cpanic=abort )
	fi

	if use cros-debug || [[ "${CROS_RUST_OVERFLOW_CHECKS}" == "1" ]]; then
		rustflags+=( -Coverflow-checks=on )
	fi

	use cros-debug && rustflags+=( -Cdebug-assertions=on )

	if use rust-coverage && [[ -z "${CROS_RUST_COVERAGE_DISABLED}" ]]; then
		# TODO(b/215596245) Use rust-coverage use flag for rust packages.
		rustflags+=( -Cinstrument-coverage )
	else
		# Remap source directories because of the following:
		# * crashes from panics are grouped across different boards
		# * the remapped strings are shorter resulting in smaller binaries
		# NOTE: this is disabled with code coverage enabled since it is
		#   incompatible.
		rustflags+=(
			# This shouldn't be needed because cargo includes local sources
			# with relative paths, but just-in-case remap the source directory.
			"--remap-path-prefix=${S}=[${PN}]"
			# Remap the cros_rust_registry/registry directory.
			"--remap-path-prefix=${SYSROOT}${CROS_RUST_REGISTRY_INST_DIR}=[REGISTRY]"
			# Remap the target directory for generated sources.
			"--remap-path-prefix=${CARGO_TARGET_DIR}=[TARGET]"
		)
	fi

	# Rust compiler is not exporting the __asan_* symbols needed in
	# asan builds. Force export-dynamic linker flag to export __asan_* symbols
	# https://crbug.com/1085546
	use asan && rustflags+=( -Zsanitizer=address -Clink-arg="-Wl,-export-dynamic" )
	use lsan && rustflags+=( -Zsanitizer=leak )
	use msan && rustflags+=( -Zsanitizer=memory -Clink-arg="-Wl,--allow-shlib-undefined")
	use tsan && rustflags+=( -Zsanitizer=thread )

	# b/520451971: We've been mislinking sanitizer crates for a while.
	# Rust started diagnosing that around Rust 1.92. Until that's fixed,
	# suppress the diags.
	if use asan || use lsan || use msan || use tsan; then
		local rustc_ver="$(rust-toolchain-version | grep -oE '^[0-9]+(\.[0-9]+)*')"
		# TODO(b/521328762): Remove this check when Rust is upgraded to 1.92
		if ver_test "${rustc_ver}" -ge "1.92.0"; then
			rustflags+=( "-Cunsafe-allow-abi-mismatch=sanitizer" )
		fi
	fi

	# We have -fsanitize=vptr enabled by default, which requires special
	# C++ sanitizer libraries to be linked in. These are not included by default
	# when linking with `clang`, so a few extra link args need to be added.
	if use ubsan; then
		rustflags+=(
			# - Specifying -fsanitize=undefined causes C ubsan libraries to be linked in.
			# - Specifying -fsanitize=vptr and --driver-mode=g++ cause the C++ ubsan
			#   libraries to be linked in.
			"-Clink-arg=--driver-mode=g++"
			"-Clink-arg=-fsanitize=undefined,vptr"
			# The C++ ubsan libraries depend on libcxx, so ensure that gets linked in.
			"-Clink-arg=-lc++"
		)
	fi

	if use fuzzer; then
		rustflags+=(
			# We can get segfaults unless we turn this off; see
			# https://github.com/rust-lang/rust/issues/99886
			# Presumably we can remove this once that bug is
			# resolved.
			-Cllvm-args=-experimental-debug-variable-locations=0
			--cfg fuzzing
			-Cpasses=sancov-module
			-Cllvm-args=-sanitizer-coverage-level=4
			-Cllvm-args=-sanitizer-coverage-inline-8bit-counters
			-Cllvm-args=-sanitizer-coverage-trace-compares
			-Cllvm-args=-sanitizer-coverage-pc-table
			-Cllvm-args=-sanitizer-coverage-trace-divs
			-Cllvm-args=-sanitizer-coverage-trace-geps
			-Cllvm-args=-sanitizer-coverage-prune-blocks=0
			-Clink-arg="-Wl,--no-gc-sections"
		)
	fi

	# Add EXTRA_RUSTFLAGS to the current rustflags. This lets us emerge rust
	# packages with locally exported flags for testing purposes as:
	# `EXTRA_RUSTFLAGS="<flags>" emerge-$BOARD <package>`
	# We want to split the flags since it's a command line as a scalar.
	# shellcheck disable=SC2206
	rustflags+=( ${EXTRA_RUSTFLAGS:=} )

	# Ensure RUSTFLAGS is *not* set in the environment.
	# If it is, it will override the flags we configure below. See:
	# https://doc.rust-lang.org/cargo/reference/config.html#buildrustflags
	# Ebuilds should set their custom rustflags in cargo configuration.
	# Developers can pass EXTRA_RUSTFLAGS for one-off builds as above.
	unset RUSTFLAGS

	# Rustc removed libstd.so from the top level /lib path because it's supposed to be
	# used only by internal tools like clippy, gated behind the rustc_private feature.
	# External users like cargo, proc_macros  must use the target-libdir for libstd.so
	# Details: https://github.com/rust-lang/rust/issues/141958
	export LD_LIBRARY_PATH="$(rustc --print target-libdir):${LD_LIBRARY_PATH}"

	# Add rustflags to the cargo configuration.
	# This 'cfg(all())' [target] section will apply to *all* targets, CHOST
	# and CBUILD.
	# TODO(b/330537088): some flags above are not applicable to all targets,
	# they should be configured into suitable [target] sections.
	local rustflags_list=$(printf "    %s,\n" "${rustflags[@]@Q}")
	local rustflags_chost_list=""
	if [[ "${#rustflags_chost[@]}" -ne 0 ]]; then
		rustflags_chost_list=$(printf "    %s,\n" "${rustflags_chost[@]@Q}")
	fi
	cat <<- EOF >> "${_CROS_RUST_CONFIG_TOML}" || die

	[target.'cfg(all())']
	rustflags = [
	${rustflags_list}
	]

	[target.${CHOST}]
	linker = "$(tc-getCC)"
	rustflags = [
	${rustflags_chost_list}
	]
	EOF
}

# @FUNCTION: cros-rust_update_cargo_lock
# @DESCRIPTION:
# Regenerates/removes the Cargo.lock file to ensure cargo uses the dependency
# versions from our local registry, and checks the rustc version to make sure
# intermediates aren't mixed across rustc versions.
cros-rust_update_cargo_lock() {
	debug-print-function "${FUNCNAME[0]}"

	if [[ -n "${CROS_WORKON_PROJECT}" ]]; then
		# Force an update the Cargo.lock file.
		ecargo generate-lockfile
		# Shellcheck thinks CROS_WORKON_INCREMENTAL_BUILD is never
		# defined.
		# shellcheck disable=SC2154
		if [[ "${CROS_WORKON_INCREMENTAL_BUILD}" == "1" ]]; then
			local previous_lockfile="${CARGO_TARGET_DIR}/Cargo.lock.prev"
			local previous_rustc="${CARGO_TARGET_DIR}/rustc.ver"
			local rustc_ver="$(rust-toolchain-version)"
			# If any of the dependencies have changed, clear the incremental results.
			if [[ ! -f "${previous_lockfile}" ]] ||
					[[ ! -f "${previous_rustc}" ]] ||
					[[ "$(< "${previous_rustc}")" != "${rustc_ver}" ]] ||
					! cmp Cargo.lock "${previous_lockfile}" ; then
				# This will print errors for the .crate files, but that is OK.
				rm -rf "${CARGO_TARGET_DIR}"
				mkdir -p "${CARGO_TARGET_DIR}"
				cp Cargo.lock "${previous_lockfile}" || die
				echo "${rustc_ver}" > "${previous_rustc}" || die
			fi
		fi
	else
		# Remove 3rd party lockfiles.
		rm -f Cargo.lock
	fi
}

# @FUNCTION: cros-rust_src_configure
# @DESCRIPTION:
# Configures the source and exports any environment variables needed during the
# build.
cros-rust_src_configure() {
	debug-print-function "${FUNCNAME[0]}"
	cros-rust_configure_cargo
	cros-rust_update_cargo_lock

	if _cros-rust_enable_edition_checks; then
		local message
		message="$(cd "${S}" && rust-edition-checker)" || die
		[[ -n "${message}" ]] && ewarn "${message}"
	fi

	default
}

# @FUNCTION: cros-rust_use_sanitizers
# @DESCRIPTION:
# Checks whether sanitizers are being used.
cros-rust_use_sanitizers() {
	use_sanitizers || use lsan
}

# @FUNCTION: ecargo
# @USAGE: <args to cargo>
# @DESCRIPTION:
# Call cargo with the specified command line options. This is an error to call
# if CROS_RUST_INSTALL_SOURCES_ONLY is set, unless it's called during the
# src_test phase func.
ecargo() {
	if [[ -n "${CROS_RUST_INSTALL_SOURCES_ONLY}" && "${EBUILD_PHASE_FUNC}" != "src_test" ]]; then
		die "CROS_RUST_INSTALL_SOURCES_ONLY packages may only call cargo from src_test."
	fi
	local args=( "$@" )
	debug-print-function "${FUNCNAME[0]}" "${args[@]}"

	addwrite Cargo.lock

	echo cargo -v "${args[@]}" >&2
	(
		# Acquire a shared (read only) lock since this does not modify
		# the registry.
		_cros-rust_acquire_fd_lock 100 --shared
		cargo -v "${args[@]}" || die
	) 100<"$(cros-rust_get_reg_lock)"
}

# @FUNCTION: write_clippy
# @INTERNAL
# @DESCRIPTION:
# Executes cargo clippy and writes lints to file
_ecargo_write_clippy() {
	# TODO(crbug.com/1194200): we should stop using /tmp for this sort of thing
	local clippy_output_base="/tmp/cargo_clippy/${CATEGORY}"
	mkdir -p "${clippy_output_base}"

	# FIXME(crbug.com/1195313): rustc sysroot may not contain dependencies
	local sysroot_old="${SYSROOT}"
	SYSROOT=$(rustc --print sysroot)
	echo "{\"package_path\":\"${S}\"}" > "${clippy_output_base}/${PF}.json"
	ecargo clippy --message-format json --target="${CHOST}" --release \
		--manifest-path="${S}/Cargo.toml" >> "${clippy_output_base}/${PF}.json"
	export SYSROOT="${sysroot_old}"
}

# @FUNCTION: ecargo_build
# @USAGE: <args to cargo build>
# @DESCRIPTION:
# Call `cargo build` with the specified command line options.
ecargo_build() {
	ecargo build --target="${CHOST}" --release "$@"
	# FIXME(b/191687433): refactor ENABLE_RUST_CLIPPY to be easier to enable/disable then remove the platform2 check
	if [[ -n "${ENABLE_RUST_CLIPPY}" && "${CROS_WORKON_PROJECT}" == "chromiumos/platform2" ]]; then
		_ecargo_write_clippy
	fi
}

# @FUNCTION: ecargo_build_fuzzer
# @DESCRIPTION:
# Call `cargo build` with fuzzing options enabled.
ecargo_build_fuzzer() {
	local fuzzer_libdir="$(dirname "$($(tc-getCC) -print-libgcc-file-name)")"
	local fuzzer_arch="${ARCH}"
	if [[ "${ARCH}" == "amd64" ]]; then
		fuzzer_arch="x86_64"
	fi

	local link_args=(
		-Clink-arg="-L${fuzzer_libdir}"
		-Clink-arg="-lclang_rt.fuzzer-${fuzzer_arch}"
		-Clink-arg="-lc++"
		-Clink-arg="-Wl,-export-dynamic"
	)

	# The `rustc` subcommand for cargo allows us to set some extra flags for
	# the current package without setting them for all `rustc` invocations.
	# On the other hand the flags in the RUSTFLAGS environment variable are set
	# for all `rustc` invocations.
	ecargo rustc --target="${CHOST}" --release "$@" -- "${link_args[@]}"
}

# @FUNCTION: cros_rust_platform_test_command
# @USAGE: <action> <bin> [-- [<test-args> ...]]
# @DESCRIPTION:
# Prints the platform2_test.py command line to execute the specified test binary
cros_rust_platform_test_command() {
	local platform2_test_py="${CHROOT_SOURCE_ROOT}/src/platform2/common-mk/platform2_test.py"

	local action="$1"
	local bin="$2"
	if [[ "$#" -gt 2 && "$3" != "--" ]]; then
		die "Need to use -- to separate program args"
	fi

	local cmd=(
		"${platform2_test_py}"
		--action="${action}"
		--strategy=unprivileged
		--user=root
	)

	if use cros_host || has "${bin}" "${CROS_RUST_HOST_TESTS[@]}"; then
		cmd+=(
			"--host"
			--sysroot="${BROOT}"
		)
	else
		cmd+=( --sysroot="${SYSROOT}" )
	fi

	cmd+=( "${CROS_RUST_PLATFORM_TEST_ARGS[@]}" )

	if [[ -n "${bin}" ]]; then
		# $3 is "--" and anything that follows is passed to the test.
		cmd+=(
			"--"
			"${bin}"
			"${@:4}"
		)
	fi
	printf "%q " "${cmd[@]}"
}

# @FUNCTION: ecargo_test
# @USAGE: <args to cargo test>
# @DESCRIPTION:
# Call `cargo test` with the specified command line options.
ecargo_test() {
	local test_dir="${CARGO_TARGET_DIR}/ecargo-test"
	local profile_flag=""
	if ! has "--profile" "$@"; then
		profile_flag="--release"
	fi
	if has "--no-run" "$@"; then
		debug-print-function ecargo test --target="${CHOST}" --target-dir \
			"${test_dir}" "${profile_flag}" "$@"
		ecargo test --target="${CHOST}" --target-dir \
			"${test_dir}" "${profile_flag}" "$@"
	else
		cros-rust_get_test_executables "$@"

		local x=0
		for (( x = 0; x <= $#; x++ )); do
			if [[ ${!x} == "--" ]]; then
				break
			fi
		done
		local test_args=( "${@:x}" )

		# Make sure there is a separator before --test-threads.
		if [[ "${#test_args[@]}" == 0 ]]; then
			test_args=( -- )
		fi

		# Limit the number of test threads if they are not limited already.
		if [[ " ${test_args[*]}" != *" --test-threads"* ]]; then
			test_args+=( "--test-threads=$(makeopts_jobs)" )
		fi

		local jobs=1
		if [[ "${CROS_RUST_TEST_MULTIPROCESS}" == "yes" ]]; then
			jobs="$(makeopts_jobs)"
		fi

		local testfile
		for testfile in "${CROS_RUST_TESTS[@]}"; do
			cros_rust_platform_test_command run "${testfile}" "${test_args[@]}"
			# Print a NUL delimiter to separate each command.
			printf "\0"
		done | xargs -0 -P "${jobs}" --verbose -I '{}' bash -x -c "{}" || die
	fi
}

# @FUNCTION: cros-rust_get_test_executables
# @USAGE: <args to cargo test>
# @DESCRIPTION:
# Call `ecargo_test` with '--no-run' and '--message-format=json' arguments.
# Then, use jq to parse and store all the test executables in a global array.
cros-rust_get_test_executables() {
	# Make sure all the targets are built before generating the json. This ensures
	# any error messages will not be hidden.
	ecargo_test --no-run "$@" || die

	mapfile -t CROS_RUST_TESTS < \
		<(ecargo_test --no-run --message-format=json "$@" | \
		jq -r 'select(.profile.test == true) | .filenames[]')

	# Cargo puts tests not compiled for the SYSROOT in ecargo-test/release.
	if [[ -z "${CROS_RUST_HOST_TESTS}" ]]; then
		local testfile
		for testfile in "${CROS_RUST_TESTS[@]}"; do
			if [[ "${testfile}" == "${CARGO_TARGET_DIR}/ecargo-test/release"* ]]; then
				CROS_RUST_HOST_TESTS+=( "${testfile}" )
			fi
		done
	fi
}

# @FUNCTION: cros-rust_get_host_test_executables
# @USAGE: <args to cargo test>
# @DESCRIPTION:
# Call `ecargo_test` with '--no-run' and '--message-format=json' arguments.
# Then, use jq to parse and store the test executables in a global array.
cros-rust_get_host_test_executables() {
	mapfile -t CROS_RUST_HOST_TESTS < \
		<(ecargo_test --no-run --message-format=json "$@" | \
		jq -r 'select(.profile.test == true) | .filenames[]')
}

# @FUNCTION: cros-rust_publish
# @USAGE: [crate name] [crate version]
# @DESCRIPTION:
# Install a library crate to the local registry store.  Should only be called
# from within a src_install() function.
# This triggers a linter error SC2120 which says:
#   "rust_publish references arguments, but none are ever passed"
# In this case, we will use without arguments to get a default value, but other
# usages exist in other files that do use arguments, so there is no problem.
# shellcheck disable=SC2120
cros-rust_publish() {
	debug-print-function "${FUNCNAME[0]}" "$@"

	[[ -n "${CROS_RUST_PREINSTALLED_REGISTRY_CRATE}" ]] && \
		die "cros-rust_publish should not be called for preinstalled registry crates"

	if [[ "${EBUILD_PHASE_FUNC}" != "src_install" ]]; then
		die "${FUNCNAME[0]}() should only be used in src_install() phase"
	fi

	local default_version="${CROS_RUST_CRATE_VERSION}"
	if [[ "${default_version}" == "9999" ]]; then
		# This triggers a linter error SC2119 which says:
		#   "Use foo "$@" if function's $1 should mean script's $1"
		# In this case, cros-rust_get_crate_version without arguments retrieves the
		# default value which is desired, so this warning can be ignored.
		# shellcheck disable=SC2119
		default_version="$(cros-rust_get_crate_version)"
	fi

	local name="${1:-${CROS_RUST_CRATE_NAME}}"
	local version="${2:-${default_version}}"

	# Cargo.toml.orig is now reserved by `cargo package`.
	if [[ -e Cargo.toml.orig ]]; then
		# Don't try to delete it if it isn't present, because that can
		# be a permission error in the Portage sandbox.
		rm -f Cargo.toml.orig || die
	fi

	if [[ -n "${CROS_WORKON_PROJECT}" ]]; then
		[[ -e "${FILESDIR}/chromeos-version.sh" ]] || die \
			"Missing chromeos-version.sh. Please add one for installation to work properly."
	fi

	local registry_dir="${D}/${CROS_RUST_REGISTRY_DIR}"
	local dir="${name}-${version}"

	# Now create a dir sort-of-as-if `cargo package` were used. `cargo
	# package` cannot be used directly, since it requires all dependencies
	# to be in the registry, and some ebuilds install multiple crates with
	# dependency edges between them.
	#
	# Few subtle bits:
	#  - symlinks inside of the dir are dereferenced, as they likely point
	#    to files inside of ${WORKDIR}.
	#  - hidden files are ignored, as they're overwhelmingly skipped by
	#    `cargo package`, and some projects have hidden broken symlinks
	#    (e.g., `.rustfmt.toml`) in their workdirs.
	mkdir -p "${registry_dir}/${dir}" || die
	find . ! -name '.*' \( -type f -or -type l \) -print0 | \
		xargs -I"{}" -0 cp --parents --dereference --preserve=mode "{}" "${registry_dir}/${dir}" || die
	pushd "${registry_dir}" > /dev/null || die

	local checksum="${T}/${name}-${version}-checksum.json"

	# Calculate the sha256 hashes of all the files in the crate. Sort the
	# files for determinism.
	# This triggers a linter error SC2207 which says:
	#   "Prefer mapfile or read -a to split command
	#    output (or quote to avoid splitting)."
	# In this case, cros-rust_get_crate_version no argument retrieves the
	# default value which is desired, so this warning can be ignored.
	# shellcheck disable=SC2207
	local files=( $(find "${dir}" -type f | sort) )

	[[ "${#files[@]}" == "0" ]] && die "Could not find crate files for ${name}"

	# Now start filling out the checksum file.

	# Synthesized package SHA sum (Cargo gets angry if it doesn't exist, but
	# doesn't care otherwise).
	local shasum="0000000000000000000000000000000000000000000000000000000000000000"
	printf '{\n\t"package": "%s",\n\t"files": {\n' "${shasum}" > "${checksum}"
	local idx=0
	local f
	for f in "${files[@]}"; do
		shasum="$(sha256sum "${f}" | cut -d ' ' -f 1)"
		printf '\t\t"%s": "%s"' "${f#"${dir}"/}" "${shasum}" >> "${checksum}"

		# The json parser is unnecessarily strict about not allowing
		# commas on the last line so we have to track this ourselves.
		idx="$((idx+1))"
		if [[ "${idx}" == "${#files[@]}" ]]; then
			printf '\n' >> "${checksum}"
		else
			printf ',\n' >> "${checksum}"
		fi
	done
	printf "\t}\n}\n" >> "${checksum}"
	popd > /dev/null || die

	insinto "${CROS_RUST_REGISTRY_DIR}/${name}-${version}"
	newins "${checksum}" .cargo-checksum.json

	# Symlink the 9999 version to the version installed by the crate.
	if [[ "${CROS_RUST_CRATE_VERSION}" == "9999" && "${version}" != "9999" ]]; then
		dosym "${name}-${version}" "${CROS_RUST_REGISTRY_DIR}/${name}-9999"
	fi
}

# @FUNCTION: cros-rust_get_build_dir
# @DESCRIPTION:
# Return the path to the directory where build artifacts are available.
cros-rust_get_build_dir() {
	echo "${CARGO_TARGET_DIR}/${CHOST}/release"
}

# @FUNCTION: cros_rust_is_direct_exec
# @DESCRIPTION:
# Return true if the compiled executables are expected to run on this platform.
cros_rust_is_direct_exec() {
	use amd64 || use x86
}

cros-rust_src_compile() {
	debug-print-function "${FUNCNAME[0]}" "$@"
	# Skip non cros-workon packages.
	[[ -z "${CROS_WORKON_PROJECT}" ]] && return 0

	ecargo_build "$@"
}

if [[ "${CROS_RUST_TEST_DIRECT_EXEC_ONLY}" == "yes" ]]; then
	RESTRICT="!x86? ( !amd64? ( test ) )"
fi

cros-rust_src_test() {
	debug-print-function "${FUNCNAME[0]}" "$@"

	eval "$(cros_rust_platform_test_command "pre_test")"
	ecargo_test "$@"
}

cros-rust_src_install() {
	debug-print-function "${FUNCNAME[0]}" "$@"

	# This triggers a linter error SC2119 which says:
	#   "Use cros-rust_publish "$@" if function's $1 should mean script's $1"
	# Here we will use without arguments to get a default value so there is no problem
	# shellcheck disable=SC2119
	cros-rust_publish
}

# @FUNCTION: _cros-rust_prepare_lock
# @INTERNAL
# @DESCRIPTION:
# Create registry lock files. This should only be called inside pkg_* functions
# to ensure the lock file is owned by root. The permissions are set to 644 so
# that $PORTAGE_USERNAME:portage will be able to obtain a shared lock inside
# src_* functions.
_cros-rust_prepare_lock() {
	if [[ "$(id -u)" -ne 0 ]]; then
		die "_cros-rust_prepare_lock should only be called inside pkg_* functions."
	fi
	local lock="$(cros-rust_get_reg_lock)"
	local dir="$(dirname "${lock}")"
	mkdir -p "${dir}" || die
	chmod 755 "${dir}" || die
	touch "${lock}" || die
	chmod 644 "${lock}" || die
}

# @FUNCTION: _cleanup_registry_link
# @INTERNAL
# @USAGE: force [crate name] [crate version]
# @DESCRIPTION:
# Unlink a library crate from the local registry. This is repeated in the prerm
# and preinst stages. If force is nonempty, the link will be cleaned up
# regardless of declared ownership. Otherwise, ownership will be respected.
_cleanup_registry_link() {
	local force="$1"
	local name="${2:-${CROS_RUST_CRATE_NAME}}"
	local version="${3:-${CROS_RUST_CRATE_VERSION}}"
	local crate="${name}-${version}"

	local crate_dir="${ROOT}${CROS_RUST_REGISTRY_DIR}/${crate}"
	if [[ "${version}" == "9999" && -L "${crate_dir}" ]]; then
		crate="$(basename "$(readlink -f "${crate_dir}")")"
	fi

	local registry_dir="${ROOT}${CROS_RUST_REGISTRY_INST_DIR}"
	local link="${registry_dir}/${crate}"
	# Add a check to avoid spamming when it doesn't exist (e.g. binary crates).
	if [[ -L "${link}" ]]; then
		# Acquire a exclusive lock since this modifies the registry.
		_cros-rust_prepare_lock
		(
			local owner="${ROOT}${CROS_RUST_REGISTRY_OWNER_DIR}/${crate}"
			local removed
			_cros-rust_acquire_fd_lock 100 --exclusive
			if [[ -n ${force} ]] || [[ $(< "${owner}") == "${PF}" ]]; then
				rm -f "${link}" "${owner}" || die
				removed=1
			fi
			flock -u 100 || die

			if [[ -n "${removed}" ]]; then
				einfo "Removed ${crate} from Cargo registry"
			else
				einfo "${crate} removal from Cargo registry" \
					"skipped due to new symlink owner"
			fi
		) 100<"$(cros-rust_get_reg_lock)"
	fi
}

# @FUNCTION: _create_registry_link
# @INTERNAL
# @USAGE: [crate name] [crate version]
# @DESCRIPTION:
# Link a library crate from the local registry. This is performed in the
# postinst stage.
_create_registry_link() {
	local name="${1:-${CROS_RUST_CRATE_NAME}}"
	local version="${2:-${CROS_RUST_CRATE_VERSION}}"
	local crate="${name}-${version}"

	local crate_dir="${ROOT}${CROS_RUST_REGISTRY_DIR}/${crate}"
	local registry_dir="${ROOT}${CROS_RUST_REGISTRY_INST_DIR}"

	if [[ "${version}" == "9999" && -L "${crate_dir}" ]]; then
		crate_dir="$(readlink -f "${crate_dir}")"
		crate="$(basename "${crate_dir}")"
	fi

	# Only install the link if there is a library crates to register. This
	# avoids dangling symlinks in the case that this only installs
	# executables.
	if [[ -e "${crate_dir}" ]]; then
		local owners_dir="${ROOT}${CROS_RUST_REGISTRY_OWNER_DIR}"
		einfo "Linking ${crate} into Cargo registry at ${registry_dir}"
		mkdir -p "${registry_dir}" "${owners_dir}"
		# A redundant link presence check is used inside the lock
		# because we do not want to lock if we don't have to, but there
		# is a time-of-check to time-of-use issue that shows up if the
		# link presence check is not in the lock (two ebuilds may try to
		# create the same lock with one succeeding and the other failing
		# because the link already exists).
		(
			local dest="${registry_dir}/${crate}"
			local owners="${owners_dir}/${crate}"
			_cros-rust_acquire_fd_lock 100 --exclusive
			if [[ ! -L "${dest}" ]]; then
				ln -srT "${crate_dir}" "${dest}" || die
			fi
			echo -n "${PF}" > "${owners}" || die
		) 100<"$(cros-rust_get_reg_lock)"
	fi
}

# @FUNCTION: cros-rust_cleanup_vendor_registry_links
# @DESCRIPTION: force [crate name ...]
# This cleans up the given vendor directories. If force is nonempty, their links
# will be cleaned up regardless of declared ownership. Otherwise, ownership will
# be respected.
cros-rust_cleanup_vendor_registry_links() {
	local force="$1"
	shift
	local dirs=( "$@" )

	local owner_dir="${ROOT}${CROS_RUST_REGISTRY_OWNER_DIR}"
	# The registry might not exist. In that case, great; skip everything.
	# Check the owner dir rather than the registry dir, since the registry
	# dir is created before the owner dir, and both are needed for the logic
	# below.
	[[ -e "${owner_dir}" ]] || return 0

	local dir remove_paths=()
	for dir in "${dirs[@]}"; do
		remove_paths+=( "${dir##*/}" )
	done

	(
		local owned_files=()

		cd "${owner_dir}" || die
		_cros-rust_acquire_fd_lock 100 --exclusive
		if [[ -n "${force}" ]]; then
			owned_files=( "${remove_paths[@]}" )
		else
			for path in "${remove_paths[@]}"; do
				if [[ "$(< "${path}" 2>/dev/null)" == "${PF}" ]]; then
					owned_files+=( "${path}" )
				fi
			done
		fi

		rm -f "${owned_files[@]}" || die
		cd "${ROOT}${CROS_RUST_REGISTRY_INST_DIR}" || die
		rm -f "${owned_files[@]}" || die
	) 100<"$(cros-rust_get_reg_lock)"
}

# @FUNCTION: cros-rust_create_vendor_registry_links
# @DESCRIPTION: [crate name ...]
# creates a registry link for every crate in [vendor tree base]. [vendor tree
# base] should be a path to the root of a Cargo vendor/ directory. Intended
# specifically for use by third-party-crates-src.
#
# This assumes that all of the crates in [vendor tree base] have been installed
# in the registry directory.
cros-rust_create_vendor_registry_links() {
	local dirs=( "$@" )

	# If the registry itself doesn't exist, portage has masked
	# installations to it (e.g., we're in `build_image`, and installing
	# registry symlinks is useless). Skip it.
	[[ -e "${ROOT}${CROS_RUST_REGISTRY_DIR}" ]] || return 0

	local registry_dir="${ROOT}${CROS_RUST_REGISTRY_INST_DIR}"
	local owner_dir="${ROOT}${CROS_RUST_REGISTRY_OWNER_DIR}"
	mkdir -p "${registry_dir}" "${owner_dir}" || die

	# Use a subshell so we can conveniently lock the registry lock only once.
	(
		local crate_srcs="${ROOT}${CROS_RUST_REGISTRY_DIR}"
		local crate crate_src
		local crates_dne=()
		_cros-rust_acquire_fd_lock 100 --exclusive
		for crate in "${dirs[@]}"; do
			crate_src="${crate_srcs}/${crate}"
			# Ensure crates exist prior to creating links. These
			# should always exist.
			if [[ -e "${crate_src}" ]]; then
				ln -srTf "${crate_src}" "${registry_dir}/${crate}" || die
				echo "${PF}" > "${owner_dir}/${crate}" || die
			else
				crates_dne+=( "${crate_src}" )
			fi
		done

		if [[ "${#crates_dne[@]}" -ne 0 ]]; then
			die "Created links with crates that DNE: ${crates_dne[*]}"
		fi
	) 100<"$(cros-rust_get_reg_lock)"
}

# @FUNCTION: cros-rust_pkg_preinst
# @USAGE: [crate name] [crate version]
# @DESCRIPTION:
# Make sure a library crate isn't linked in the local registry prior to the
# install step to avoid races.
cros-rust_pkg_preinst() {
	[[ -n "${CROS_RUST_PREINSTALLED_REGISTRY_CRATE}" ]] && return
	debug-print-function "${FUNCNAME[0]}" "$@"

	if [[ "${EBUILD_PHASE_FUNC}" != "pkg_preinst" ]]; then
		die "${FUNCNAME[0]}() should only be used in pkg_preinst() phase"
	fi

	# Forcibly remove any existing link.
	_cleanup_registry_link 1 "$@"
}

# @FUNCTION: cros-rust_pkg_postinst
# @USAGE: [crate name] [crate version]
# @DESCRIPTION:
# Install a library crate in the local registry store into the registry,
# making it visible to Cargo.
cros-rust_pkg_postinst() {
	debug-print-function "${FUNCNAME[0]}" "$@"
	[[ -n "${CROS_RUST_PREINSTALLED_REGISTRY_CRATE}" ]] && return

	if [[ "${EBUILD_PHASE_FUNC}" != "pkg_postinst" ]]; then
		die "${FUNCNAME[0]}() should only be used in pkg_postinst() phase"
	fi

	_create_registry_link "$@"
}

# @FUNCTION: cros-rust_pkg_prerm
# @USAGE: [crate name] [crate version]
# @DESCRIPTION:
# Unlink a library crate from the local registry unless another package now owns
# the link.
cros-rust_pkg_prerm() {
	debug-print-function "${FUNCNAME[0]}" "$@"
	[[ -n "${CROS_RUST_PREINSTALLED_REGISTRY_CRATE}" ]] && return

	if [[ "${EBUILD_PHASE_FUNC}" != "pkg_prerm" ]]; then
		die "${FUNCNAME[0]}() should only be used in pkg_prerm() phase"
	fi

	# Clean the link only if it's still owned by us
	_cleanup_registry_link "" "$@"
}

# @FUNCTION: cros-rust_get_crate_version
# @USAGE: <path to crate>
# @DESCRIPTION:
# Returns the version for a crate by finding the first 'version =' line in the
# Cargo.toml in the crate.
# This triggers a linter error SC2120 which says:
#   "rust_get_crate_version references arguments, but none are ever passed"
# In this case, we will use without arguments to get a default value, but other
# usages exist in other files that do use arguments, so there is no problem.
# shellcheck disable=SC2120
cros-rust_get_crate_version() {
	local crate="${1:-${S}}"
	[[ $# -gt 1 ]] && die "${FUNCNAME[0]}: incorrect number of arguments"
	awk '/^version = / { print $3 }' "${crate}/Cargo.toml" | head -n1 | tr -d '"'
}

fi
