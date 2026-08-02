# Copyright 2022 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

# @ECLASS: cros-rustc.eclass
# @MAINTAINER:
# The Chromium OS Toolchain Team <chromeos-toolchain@google.com>
# @BUGREPORTS:
# Please report bugs via
# https://issuetracker.google.com/issues/new?component=1038090&template=1576440
# @VCSURL: https://chromium.googlesource.com/chromiumos/overlays/chromiumos-overlay/+/HEAD/eclass/@ECLASS@
# @BLURB: Eclass for building CrOS' Rust toolchain.
# @DESCRIPTION:
# This eclass is used to build both dev-lang/rust-host and dev-lang/rust.
#
# dev-lang/rust-host is an ebuild that provides all artifacts necessary for
# building Rust for the host. dev-lang/rust supplements this with libraries for
# cross-compiling. We maintain this split because we need to build Rust
# binaries for the host prior to cross-* libraries being available.
#
# An important concept when building dev-lang/rust-host and dev-lang/rust is
# continuity: these packages are expected to be built from _identical_ Rust
# sources. This assumption:
# - doesn't restrict us in any meaningful way,
# - keeps us more consistent with upstream flows for building `rustc`, and
# - allows us to significantly cut down on the build time of dev-lang/rust,
#   since dev-lang/rust can skip unpacking sources, configuring them, and
#   rebuilding LLVM + stage0 + stage1.
#
# Moreover, if you want to make meaningful changes to Rust, you'll need to
# always reemerge _both_ dev-lang/rust-host and dev-lang/rust. dev-lang/rust
# assumes that the sources unpacked by dev-lang/rust-host, if present, are
# identical to the ones it will build. dev-lang/rust-host always starts with a
# clean slate.

if [[ -z ${_ECLASS_CROS_RUSTC} ]]; then
_ECLASS_CROS_RUSTC="1"

# Check for EAPI 7+.
case "${EAPI:-0}" in
[0123456]) die "unsupported EAPI (${EAPI}) in eclass (${ECLASS})" ;;
esac

EXPORT_FUNCTIONS pkg_setup src_unpack src_prepare src_configure src_compile

# NOTE: RUSTC_STABLE_VERSION is matched on by `chromeos-version.sh` scripts.
# Please don't deviate from it matching /^RUSTC_STABLE_VERSION="[^"]+"/.
RUSTC_STABLE_VERSION="1.90.0"
MY_P="rustc-${RUSTC_STABLE_VERSION}"
S="${WORKDIR}/${MY_P}-src"
CROS_RUSTC_BUILD_DIR="${WORKDIR}/build"
CROS_RUSTC_LLVM_SRC_DIR="${S}/src/llvm-project"

# The commit we're using for our LLVM.
CROS_WORKON_COMMIT="2916b99182752b1aece8cc4479d8d6a20b5e02da"
CROS_WORKON_TREE="fbfdaa24e61c4e41d8110a3851ddc0968bf525c6" # git rev-parse $COMMIT^{tree}
LLVM_SVN_REVISION="484197"

CROS_WORKON_PROJECT="external/github.com/llvm/llvm-project"
CROS_WORKON_LOCALNAME="llvm-project"
CROS_WORKON_OPTIONAL_CHECKOUT=(
	"use rust_cros_llvm"
)
CROS_WORKON_DESTDIR="${CROS_RUSTC_LLVM_SRC_DIR}"
CROS_WORKON_MANUAL_UPREV=1

PYTHON_COMPAT=( python3_{8..11} )
inherit cros-llvm cros-constants git-r3 python-any-r1 cros-toolchain-funcs cros-workon

# It's intended that only a person upgrading the Rust version used in ChromeOS
# needs to worry about these flags.
#
# These flags control whether to build a compiler that will generate PGO
# profiles, or build a compiler using PGO profiles obtained locally, or build a
# compiler using PGO profiles obtained from gs (the default).
#
# rust_profile_frontend_generate causes the Rust compiler to be built
# with instrumentation in the frontend code for generating PGO profiles,
# which will be stored in "${CROS_RUSTC_PGO_LOCAL_BASE}/frontend-profraw"
#
# rust_profile_llvm_generate causes the Rust compiler to be built
# with instrumentation in the LLVM code for generating PGO profiles,
# which will be stored in "${CROS_RUSTC_PGO_LOCAL_BASE}/llvm-profraw"
#
# The two *_generate flags cannot be used together; the implementation here
# asserts against this possibility. Currently if we try to instrument both
# components at once, we get an error about different profiler versions. Maybe
# this can be changed when Rust uses the same LLVM as sys-devel/llvm.
#
# rust_profile_frontend_use will cause a frontend profdata file to be
# downloaded from
# "gs://chromeos-localmirror/distfiles/rust-pgo-${RUSTC_STABLE_VERSION}-frontend.profdata.xz"
# and used for PGO optimization.
#
# rust_profile_frontend_use_local will instead use a frontend profdata file at
# ${FILESDIR}/cros-rustc/frontend.profdata
#
# rust_profile_llvm_use will cause an llvm profdata file to be downloaded from
# "gs://chromeos-localmirror/distfiles/rust-pgo-${RUSTC_STABLE_VERSION}-llvm.profdata.xz"
# and used for PGO optimization.
#
# rust_profile_llvm_use_local will instead use a llvm profdata file at
# ${FILESDIR}/cros-rustc/llvm.profdata
IUSE='rust_profile_frontend_generate rust_profile_llvm_generate rust_profile_frontend_use_local rust_profile_llvm_use_local +rust_profile_frontend_use +rust_profile_llvm_use rust_cros_llvm'

REQUIRED_USE="
	rust_profile_frontend_generate? (
		!rust_profile_frontend_use
		!rust_profile_frontend_use_local
		!rust_profile_llvm_use
		!rust_profile_llvm_use_local
	)
	rust_profile_llvm_generate? (
		!rust_profile_frontend_use
		!rust_profile_frontend_use_local
		!rust_profile_llvm_use
		!rust_profile_llvm_use_local
	)
	rust_profile_llvm_use? ( !rust_profile_llvm_use_local )
	rust_profile_frontend_use? ( !rust_profile_frontend_use_local )
"

# @ECLASS-VARIABLE: RUSTC_TARGET_TRIPLES
# @DEFAULT_UNSET
# @REQUIRED
# @DESCRIPTION:
# This is the list of target triples for rustc as they appear in the cros_sdk.
# cros-rust_src_configure instructs cros-rust_src_compile to use
# "${triple}-clang" when building each one of these.

# @ECLASS-VARIABLE: RUSTC_BARE_TARGET_TRIPLES
# @DEFAULT_UNSET
# @DESCRIPTION:
# These are the triples we use GCC with. `*-cros-*` triples should not be
# included here.

# @ECLASS-VARIABLE: CROS_RUSTC_BUILD_RAW_SOURCES
# @DEFAULT_UNSET
# @DESCRIPTION:
# Set to a nonempty value if we want to build a nonstandard set of sources
# (this is intended mostly to power bisection of rustc itself).
# This should never be set to anything in production.
#
# If you want to set this as a user, each `emerge` of `dev-lang/rust-host` or
# `dev-lang/rust` assumes the following:
# 1. A full Rust checkout is available under `_CROS_RUSTC_RAW_SOURCES_ROOT`.
# 2. You've ensured that all submodules under `_CROS_RUSTC_RAW_SOURCES_ROOT` are
#    up-to-date with your currently checked out revision.
# 3. You've ensured that the appropriate bootstrap compiler is cached under
#    `_CROS_RUSTC_RAW_SOURCES_ROOT/build`.
# 4. You've run `cargo vendor` under `_CROS_RUSTC_RAW_SOURCES_ROOT`
# 5. The sources under `_CROS_RUSTC_RAW_SOURCES_ROOT` are the exact sources you
#    want to apply `${PATCHES}` to.
# 6. You are OK with this script modifying your rustc sources at
#    `_CROS_RUSTC_RAW_SOURCES_ROOT` (by applying patches to them).
#
# Step 2 can be done with
# `dev-lang/rust/files/bisect-scripts/clean_and_sync_rust_root.sh`. Steps 3 and
# 4 can be accomplished with
# `dev-lang/rust/files/bisect-scripts/prepare_rust_for_offline_build.sh`.
CROS_RUSTC_BUILD_RAW_SOURCES=

# @ECLASS-VARIABLE: CROS_RUSTC_BOOTSTRAP_FROM_RUST_HOST
# @DEFAULT_UNSET
# @DESCRIPTION:
# If nonempty, cros-rustc will configure builds to be based on rust-host's
# builds. This has a few implications:
# - Stage2 builds are not attempted, since the _baseline stage_ of all builds is
#   conceptually stage2 from rust-host's standpoint. This is because rust-host
#   is a stage2 toolchain. Hence, stage2 builds that are bootstrapped from
#   rust-host are actually stage4 builds from rust-bootstrap's POV.
# - This package will RDEPEND and DEPEND on rust-host-${PVR}, rather than
#   DEPENDing on rust-bootstrap. Revision locking is necessary, since building
#   on rust-host's stage2 build requires the same source tree as rust-host.
# - This package will not build additional host tools (e.g., rust-analyzer), as
#   rust-host is the provider of those.

# We identify the .profdata file we want by ${RUSTC_STABLE_VERSION}. Sometimes
# we may want to upload and use a newer profdata file even if we haven't bumped
# rustc's version; these can be distinguished with this suffix.
PROFDATA_SUFFIX=""

# There's a fair amount of direct commonality between dev-lang/rust and
# dev-lang/rust-host. Capture that here.
ABI_VER="$(ver_cut 1-2)"
SLOT="stable/${ABI_VER}"
SRC="${MY_P}-src.tar.gz"

# The version of rust-bootstrap that we're using to build our current Rust
# toolchain. This is generally the version released prior to the current one,
# since Rust uses the beta compiler to build the nightly compiler.
BOOTSTRAP_VERSION="1.89.0"

# If `CROS_RUSTC_BUILD_RAW_SOURCES` is nonempty, a full Rust source tree is
# expected to be available here.
_CROS_RUSTC_RAW_SOURCES_ROOT="${FILESDIR}/cros-rustc/rust"

HOMEPAGE="https://www.rust-lang.org/"

if [[ -z "${CROS_RUSTC_BUILD_RAW_SOURCES}" ]]; then
	SRC_URI="https://static.rust-lang.org/dist/${SRC} -> rustc-${RUSTC_STABLE_VERSION}-src.tar.gz"
else
	SRC_URI=""
	# If a bisection is happening, we use the bootstrap compiler that upstream prefers.
	# Clear this so we don't accidentally use it below.
	BOOTSTRAP_VERSION=
fi

# If INCLUDE_PROFDATA_IN_SRC_URI is empty, do not include profdata
# entries in SRC_URI. This exists so that we can build the instrumented
# compiler before the profiling data exists.
#
# Note: The logic associated with this variable is intentionally minimal.
# In particular, it does not do anything intelligent to the *_profile_*
# USE flags; however, ebuilds with this variable set to empty should
# only be used to generate profile data.
INCLUDE_PROFDATA_IN_SRC_URI=yes
if [[ -n "${INCLUDE_PROFDATA_IN_SRC_URI}" ]]; then
	PROFDATA_PREFIX="gs://chromeos-localmirror/distfiles"
	SRC_URI+="
		rust_profile_frontend_use? (
			${PROFDATA_PREFIX}/rust-pgo-${RUSTC_STABLE_VERSION}${PROFDATA_SUFFIX}-frontend.profdata.xz
		)
		rust_profile_llvm_use? (
			${PROFDATA_PREFIX}/distfiles/rust-pgo-${RUSTC_STABLE_VERSION}${PROFDATA_SUFFIX}-llvm.profdata.xz
		)
	"
fi

LICENSE="|| ( MIT Apache-2.0 ) BSD-1 BSD-2 BSD-4 UoI-NCSA"

RESTRICT="binchecks strip"

DEPEND="${PYTHON_DEPS}
	>=dev-libs/libxml2-2.9.6
	>=dev-lang/perl-5.0
"

if [[ -z "${CROS_RUSTC_BUILD_RAW_SOURCES}" ]]; then
	if [[ -z "${CROS_RUSTC_BOOTSTRAP_FROM_RUST_HOST}" ]]; then
		# This should theoretically be a BDEPEND, but those aren't
		# currently respected during SDK updates:
		# https://ci.chromium.org/b/8756555875933892513
		DEPEND+="dev-lang/rust-bootstrap:${BOOTSTRAP_VERSION}"
	else
		# N.B., Using ${PVR} here is critical:
		# 1. if a package that's supposed to be source-locked with
		#    `rust-host` has been `cros-workon`'ed, `rust-host` should
		#    also be `cros-workon`'ed.
		# 2. If the revision of `rust-host` changes, that may imply that
		#    `rust-host`'s sources or config changed. The package being
		#    built needs to reflect this, as well.
		DEPEND+="=dev-lang/rust-host-${PVR}"
		RDEPEND="=dev-lang/rust-host-${PVR}"
	fi
fi

BDEPEND="
	dev-util/cmake
	dev-util/ninja
"

PATCHES=(
	"${FILESDIR}/cros-rustc/rust-force-host-triple.patch"
	"${FILESDIR}/cros-rustc/rust-add-cros-targets.patch"
	"${FILESDIR}/cros-rustc/rust-fix-rpath.patch"
	"${FILESDIR}/cros-rustc/rust-sanitizer-supported.patch"
	"${FILESDIR}/cros-rustc/rust-cc.patch"
	"${FILESDIR}/cros-rustc/rust-ld-argv0.patch"
	"${FILESDIR}/cros-rustc/rust-Handle-sparse-git-repo-without-erroring.patch"
	"${FILESDIR}/cros-rustc/rust-add-armv7a-sanitizers.patch"
	"${FILESDIR}/cros-rustc/rust-bootstrap-use-CARGO_HOME.patch"
	"${FILESDIR}/cros-rustc/rust-ignore-version-in-mangling.patch"
	"${FILESDIR}/cros-rustc/rust-rustc_llvm-stage0-is-not-cross-compiling.patch"
	"${FILESDIR}/cros-rustc/rust-allow-stage0-builds-like-stage1-with-local_rebuild.patch"
	# NOTE: When this patch is deleted, please also delete the
	# `_cros-rustc-regenerate-vendor-dir-checksums` logic below.
	"${FILESDIR}/cros-rustc/rust-teach-cc-about-our-triples.patch"
)

# b/460145304 and b/437343779#comment11: When building from rust-bootstrap, we
# need stage0-sysroot to be copied from an ever-so-slightly different
# directory, and to an ever-so-slightly different directory.
if [[ -z "${CROS_RUSTC_BOOTSTRAP_FROM_RUST_HOST:-}" ]]; then
	PATCHES+=( "${FILESDIR}/cros-rustc/rust-host-only-copy-rustlib-into-sysroot.patch" )
else
	PATCHES+=( "${FILESDIR}/cros-rustc/rust-only-copy-rustlib-into-sysroot.patch" )
fi

CROS_RUSTC_PGO_LOCAL_BASE='/tmp/rust-pgo'

cros-rustc_pkg_setup() {
	python-any-r1_pkg_setup
}

cros-rustc_src_unpack() {
	debug-print-function "${FUNCNAME[0]}" "${@}"

	if [[ -n "${CROS_RUSTC_BUILD_RAW_SOURCES}" ]]; then
		if [[ ! -d "${_CROS_RUSTC_RAW_SOURCES_ROOT}" ]]; then
			eerror "You must have a full Rust checkout in _CROS_RUSTC_RAW_SOURCES_ROOT."
			die
		fi
		if [[ -e "${S}" && ! -L "${S}" ]]; then
			rm -rf "${S}" || die
		fi
		# shellcheck disable=SC2174
		ln -sf "$(readlink -m "${_CROS_RUSTC_RAW_SOURCES_ROOT}")" "${S}" || die
		default
		return
	fi

	default

	if use rust_cros_llvm; then
		einfo "Removing vendored llvm-project..."
		rm -rf "${CROS_RUSTC_LLVM_SRC_DIR}"
		# Override S so cros-workon_enforce_subtrees doesn't call adddeny on
		# our cache directory.
		S="${CROS_RUSTC_LLVM_SRC_DIR}" cros-workon_src_unpack
	fi
}

_cros-rustc_apply_llvm_patches() {
	S="${CROS_RUSTC_LLVM_SRC_DIR}" prepare_patches "${LLVM_SVN_REVISION}"
}

_cros-rustc-regenerate-vendor-dir-checksums() {
	local vendor_dir="$1"
	(
		cd "${vendor_dir}" || die
		einfo "Fixing up vendor checksums in ${PWD}"
		local old_package_hash
		old_package_hash="$(grep -o '"package": *"[^"]\+"}$' .cargo-checksum.json)" || die
		old_package_hash="$(cut -d'"' -f4 <<< "${old_package_hash}")" || die
		rm -f .cargo-checksum.json || die
		# Forming a JSON object on the fly is hacky, but:
		# - This only needs to apply to a very specific set of files (at the time
		#   of writing, just cc/)
		# - It's hoped that this function will be deleted in the medium-term anyway.
		local new_file_contents
		local sha file_path leading_comma
		# N.B., the 'package' checksum is only checked between this file and the
		# Cargo.lock, so there's no need to regenerate it
		new_file_contents='{"package": "'"${old_package_hash}"'", "files": {'
		while read -r sha file_path; do
			# `cargo` wants `src/foo.rs`, not `./src/foo.rs`.
			file_path=${file_path#./}
			new_file_contents+="${leading_comma}\"${file_path}\": \"${sha}\""
			leading_comma=","
		done < <(find ./ -type f -print0 | xargs -P8 -0 sha256sum | sort)
		new_file_contents+="}}"
		echo "${new_file_contents}" > .cargo-checksum.json || die
	)
}

cros-rustc_src_prepare() {
	debug-print-function "${FUNCNAME[0]}" "${@}"

	if [[ -n "${CROS_RUSTC_BUILD_RAW_SOURCES}" ]]; then
		einfo "Synchronizing bootstrap compiler caches ..."
		cp -avu "${_CROS_RUSTC_RAW_SOURCES_ROOT}/build/cache" "${CROS_RUSTC_BUILD_DIR}" || die
	elif use rust_cros_llvm; then
		# TODO: LLVM has been migrated to use an actual branch with
		# patches applied directly to it.
		#
		# This USE flag is hoped to be made live in 2025-2026; when
		# someone starts putting cycles into turning it on, this should
		# serve as a reminder to update for the new LLVM flow.
		die "This configuration is out-of-date; please update it to work with LLVM's new branch flow."
	fi

	einfo "Applying Rust patches..."
	# Copy "unknown" vendor targets to create cros_sdk target triple
	# variants as referred to in rust-add-cros-targets.patch and
	# RUSTC_TARGET_TRIPLES. armv7a is treated specially because the cros
	# toolchain differs in more than just the vendor part of the target
	# triple. The arch is armv7a in cros versus armv7.
	pushd compiler/rustc_target/src/spec/targets || die
	sed -e 's:"unknown":"pc":g' x86_64_unknown_linux_gnu.rs >x86_64_pc_linux_gnu.rs || die
	sed -e 's:"unknown":"cros":g' x86_64_unknown_linux_gnu.rs >x86_64_cros_linux_gnu.rs || die
	sed -e 's:"unknown":"cros":g' armv7_unknown_linux_gnueabihf.rs >armv7a_cros_linux_gnueabihf.rs || die
	sed -e 's:"unknown":"cros":g' aarch64_unknown_linux_gnu.rs >aarch64_cros_linux_gnu.rs || die
	popd || die

	# For the rustc_llvm module, the build will link with -nodefaultlibs and
	# manually choose the std C++ library. For x86_64 Linux, the build
	# script always chooses libstdc++ which will not work if LLVM was built
	# with USE="default-libcxx". This snippet changes that choice to libc++
	# in the case that clang++ defaults to libc++.
	if "${CXX}" -### -x c++ - < /dev/null 2>&1 | grep -q -e '-lc++'; then
		sed -i 's:"stdc++":"c++":g' compiler/rustc_llvm/build.rs || die
	fi

	default
	_cros-rustc-regenerate-vendor-dir-checksums "${S}/vendor/cc-1.2.0"
	_cros-rustc-regenerate-vendor-dir-checksums "${S}/vendor/cc-1.2.16"

	einfo "Rust patch application completed successfully."
}

# Prints the absolute path to the given tool, or `die`s.
_cros-rustc-tool_abspath() {
	local cmd tool="$1"
	cmd="$(type -P "${tool}")" || die "Couldn't look up ${tool}"
	realpath --no-symlinks "${cmd}" || die
}

cros-rustc_src_configure() {
	debug-print-function "${FUNCNAME[0]}" "${@}"

	tc-export CC PKG_CONFIG

	if [[ -z "${INCLUDE_PROFDATA_IN_SRC_URI}" ]]; then
		ewarn 'INCLUDE_PROFDATA_IN_SRC_URI is empty; please only use this build to generate profiles.'
	fi

	# If FEATURES=ccache is set, we can cache LLVM builds. We could set this to
	# true unconditionally, but invoking `ccache` to just have it `exec` the
	# compiler costs ~10secs of wall time on rust-host builds. No point in
	# wasting the cycles.
	local use_ccache=false
	[[ -z "${CCACHE_DISABLE:-}" ]] && use_ccache=true

	local targets=""
	local tt
	# These variables are defined by users of this eclass; their use here is safe.
	# shellcheck disable=SC2154
	for tt in "${RUSTC_TARGET_TRIPLES[@]}" "${RUSTC_BARE_TARGET_TRIPLES[@]}" ; do
		targets+="\"${tt}\", "
	done

	local bootstrap_compiler_info
	local llvm_options
	local rust_options
	local tools

	if [[ -z "${CROS_RUSTC_BUILD_RAW_SOURCES}" ]]; then
		local bootstrap_sysroot local_rebuild
		if [[ -n "${CROS_RUSTC_BOOTSTRAP_FROM_RUST_HOST}" ]]; then
			bootstrap_sysroot="/usr"
			local_rebuild=true
		else
			bootstrap_sysroot="/opt/rust-bootstrap-${BOOTSTRAP_VERSION}"
			local_rebuild=false
		fi
		read -r -d '' bootstrap_compiler_info <<- EOF
			cargo = "${bootstrap_sysroot}/bin/cargo"
			rustc = "${bootstrap_sysroot}/bin/rustc"
			local-rebuild = ${local_rebuild}
		EOF
	fi

	read -r -d '' tools <<- EOF
		tools = ["cargo", "rustfmt", "clippy", "cargofmt", "rustdoc", "rust-analyzer"]
	EOF

	if use rust_profile_llvm_generate || use rust_profile_frontend_generate; then
		ewarn 'This build is instrumented; please only use it to generate profiles.'
		read -r -d '' tools <<- EOF
			# This is an instrumented build, only meant to generate profiles, so we don't need the other tools.
			tools = ["cargo"]
		EOF
	fi

	local llvm_use_pgo_file="${WORKDIR}/rust-pgo-${RUSTC_STABLE_VERSION}${PROFDATA_SUFFIX}-llvm.profdata"
	local frontend_use_pgo_file="${WORKDIR}/rust-pgo-${RUSTC_STABLE_VERSION}${PROFDATA_SUFFIX}-frontend.profdata"
	if use rust_profile_llvm_use_local; then
		llvm_use_pgo_file="${FILESDIR}/cros-rustc/llvm.profdata"
	fi

	if use rust_profile_frontend_use_local; then
		frontend_use_pgo_file="${FILESDIR}/cros-rustc/frontend.profdata"
	fi

	# Either of the instrumented builds will apparently build an instrumented
	# stage 1 compiler, and then use it to build an instrumented stage 2 compiler.
	if use rust_profile_llvm_generate; then
		read -r -d '' llvm_options <<- EOF
			# Without the -vp-static-alloc=false option, we get
			# LLVM Profile Warning: Unable to track new values: Running out of static counters.
			# Alternatively we could use -vp-counters-per-site=2
			# The advantage of using one over the other is unclear.
			cflags = "-fprofile-generate=${CROS_RUSTC_PGO_LOCAL_BASE}/llvm-profraw -mllvm -vp-static-alloc=false"
			cxxflags = "-fprofile-generate=${CROS_RUSTC_PGO_LOCAL_BASE}/llvm-profraw -mllvm -vp-static-alloc=false"
			link-shared = true
		EOF
	fi

	if use rust_profile_frontend_generate; then
		read -r -d '' llvm_options <<- EOF
			# Without the -vp-static-alloc=false option, we get
			# LLVM Profile Warning: Unable to track new values: Running out of static counters.
			# Alternatively we could use -vp-static-alloc=false.
			cflags = "-mllvm -vp-static-alloc=false"
			cxxflags = "-mllvm -vp-static-alloc=false"
		EOF
		read -r -d '' rust_options <<- EOF
			profile-generate = "${CROS_RUSTC_PGO_LOCAL_BASE}/frontend-profraw"
		EOF
	fi

	# For profdata, note that we gracefully handle profile nonexistence for
	# 9999 ebuilds. This is intended as a convenience feature, since otherwise,
	# `rust_uprev.py` puts us into a state where there's no PGO profile, but
	# PGO is enabled by default. This leads to build failures if you do
	# e.g., `emerge cross-x86_64-cros-linux-gnu/rust` without
	# USE='-two_long -use_flags'
	#
	# CQ builds will still fail if PGO is misconfigured (they stabilize locally instead
	# of building 9999 ebuilds). This failure still allows us to catch PGO
	# misconfiguration before changes land.
	if use rust_profile_llvm_use || use rust_profile_llvm_use_local; then
		if [[ ! -f "${llvm_use_pgo_file}" ]]; then
			[[ "${PV}" == 9999 ]] || die "No LLVM profdata file"
			eerror "No LLVM profdata file, but cros-workon detected. This build will break in CQ."
		else
			read -r -d '' llvm_options <<- EOF
				cflags = "-fprofile-use=${llvm_use_pgo_file}"
				cxxflags = "-fprofile-use=${llvm_use_pgo_file}"
			EOF
		fi
	fi

	if use rust_profile_frontend_use || use rust_profile_frontend_use_local; then
		if [[ ! -f "${frontend_use_pgo_file}" ]]; then
			[[ "${PV}" == 9999 ]] || die "No frontend profdata file"
			eerror "No frontend profdata file, but cros-workon detected. This build will break in CQ."
		else
			read -r -d '' rust_options <<- EOF
				profile-use = "${frontend_use_pgo_file}"
			EOF
		fi
	fi

	local build_llvm_tools=true
	local extended_build=true
	# No need to build llvm toolchain tools (e.g., llvm-objcopy) or rust
	# ones (e.g., cargo) if we're bootstrapping from rust-host, since
	# rust-host already has those tools.
	if [[ -n "${CROS_RUSTC_BOOTSTRAP_FROM_RUST_HOST}" ]]; then
		build_llvm_tools=false
		extended_build=false
	fi

	local config=cros-config.toml
	cat > "${config}" <<- EOF
		[build]
		# rust-bootstrap has 'host == x86_64-unknown-linux-gnu', but we
		# want our rustc to be built for CrOS' host triple.
		build = "x86_64-pc-linux-gnu"
		host = ["${CHOST}"]
		target = [${targets}]
		docs = false
		submodules = false
		python = "${EPYTHON}"
		vendor = true
		extended = ${extended_build}
		${tools}
		sanitizers = true
		profiler = true
		build-dir = "${CROS_RUSTC_BUILD_DIR}"
		${bootstrap_compiler_info}
		description = "Run /usr/bin/rust-toolchain-version for more detail"
		ccache = ${use_ccache}

		[llvm]
		ninja = true
		experimental-targets = ""
		targets = "AArch64;ARM;X86"
		static-libstdcpp = false
		${llvm_options}

		[install]
		prefix = "${ED}usr"
		libdir = "$(get_libdir)"
		mandir = "share/man"

		[rust]
		default-linker = "${CBUILD}-clang"
		channel = "nightly"
		# b/271569975: codegen-units feed into cargo's 'profile' for
		# libraries. Differing 'profile's lead to incompatible build
		# artifacts (since cargo's 'profile' generally consists of
		# things like whether the build is debug/release/etc). We need
		# _some_ consistent value here.
		#
		# Pick 32 because that's what we've been shipping from the SDK
		# builder for a while.
		codegen-units = 32
		llvm-libunwind = 'in-tree'
		codegen-tests = false
		new-symbol-mangling = true
		# b/408239468#comment6: thin-local is required here because 'thin' requires
		# Rust to use its built-in LLVM to parse ThinLTO'ed '.a' files generated with
		# sys-devel/llvm. This can lead to errors. If we 'link up' this Rust with
		# the LLVM provided by sys-devel/llvm, we can probably swap back to 'thin'
		# here.
		lto = "thin-local"
		llvm-tools = ${build_llvm_tools}
		${rust_options}
	EOF

	# Ensure that CHOST always has target defs for cc/cxx/linker.
	local extra_target_triples=( "${CHOST}" )
	for tt in "${RUSTC_TARGET_TRIPLES[@]}"; do
		if [[ "${tt}" == "${CHOST}" ]]; then
			extra_target_triples=()
			break
		fi
	done

	local ar ranlib
	# These need to be absolute paths for Rust to accept them; see
	# discussion on b/277968325 comments 6-10.
	ar="$(_cros-rustc-tool_abspath "$(tc-getBUILD_AR)")"
	ranlib="$(_cros-rustc-tool_abspath "$(tc-getBUILD_RANLIB)")"
	for tt in "${extra_target_triples[@]}" "${RUSTC_TARGET_TRIPLES[@]}" ; do
		cat >> "${config}" <<- EOF
			[target."${tt}"]
			ar = "${ar}"
			cc = "${tt}-clang"
			cxx = "${tt}-clang++"
			linker = "${tt}-clang++"
			ranlib = "${ranlib}"
		EOF
	done

	# Set profiler = false for RUSTC_BARE_TARGET_TRIPLES, because the
	# header files etc. needed to build the profiler code don't exist
	# for those targets.
	for tt in "${RUSTC_BARE_TARGET_TRIPLES[@]}"; do
		cat >> "${config}" <<- EOF
			[target."${tt}"]
			profiler = false
			sanitizers = false
		EOF
	done
}

cros-rustc_src_compile() {
	debug-print-function "${FUNCNAME[0]}" "${@}"

	local stage=2
	# If we're bootstrapping from rust-host, only stage 1 is necessary.
	#
	# TODO(b/460404665): Stage 0, as of Rust 1.89, just copies existing
	# rlibs from the host, so stage 1 is needed here. Stage 1 makes
	# `cross-*/rust` take 1.5x longer to `emerge`. With Rust 1.91, we should
	# regain the ability to use stage=0 here.
	[[ -n "${CROS_RUSTC_BOOTSTRAP_FROM_RUST_HOST}" ]] && stage=1
	${EPYTHON} x.py build --stage "${stage}" --config cros-config.toml "$@" || die
}
fi
