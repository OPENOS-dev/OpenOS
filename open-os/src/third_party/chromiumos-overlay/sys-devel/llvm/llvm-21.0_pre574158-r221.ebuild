# Copyright 1999-2015 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Id$

EAPI=7

CROS_WORKON_COMMIT="686356bf458d2441c36d62ce8f047486329ec666"
CROS_WORKON_TREE="85edb12d8c585841ab8052c7d69ecaff50aeafe8"
PYTHON_COMPAT=( python3_{8..11} )

inherit cros-constants

CROS_WORKON_REPO="${CROS_GIT_HOST_URL}"
CROS_WORKON_PROJECT="external/github.com/llvm/llvm-project"
CROS_WORKON_LOCALNAME="llvm-project"
CROS_WORKON_EGIT_BRANCH="main"
CROS_WORKON_OUTOFTREE_BUILD="1"

# Build somewhat differently when a dev is iterating on LLVM.
if [[ "${PV}" == "9999" ]]; then
	# Use incremental builds only for 9999 ebuilds, since caching in the
	# face of toolchain updates can get subtle, and non-9999 builders
	# (mostly bots) won't benefit from keeping cache artifacts around.
	CROS_WORKON_INCREMENTAL_BUILD="1"
	# Exempt this package from stripping, since many LLVM binaries will try
	# to self-inspect to provide a helpful backtrace upon crashing.
	# Stripping Clang leads to that backtrace having no symbols.
	RESTRICT="strip"
fi

inherit cros-workon cros-remoteexec cmake flag-o-matic git-r3 multilib-minimal  \
	python-single-r1 pax-utils toolchain-funcs cros-llvm


DESCRIPTION="Low Level Virtual Machine"
HOMEPAGE="http://llvm.org/"

# LLVM_PGO_PROFILE_REVS contains an LLVM revision for each PGO profile that we
# ship. Revisions *may* have custom version suffixes, separated by a `-`. For
# example, "516547-v2" is a profile for r516547, with the suffix "v2". There
# should not be multiple equal revisions here (ignoring suffixes).
#
# NOTE(b/334876457): This array is managed automatically. If you land any
# changes, they're likely to be overwritten.
# toolchain-utils/pgo_tools/auto_update_llvm_pgo_profile.py may have helpful
# pointers.
LLVM_PGO_PROFILE_REVS=(
	574158
	584947
	596125
)

populate_pgo_profile_urls() {
	LLVM_PGO_PROFILE_URLS=
	local rev
	for rev in "${LLVM_PGO_PROFILE_REVS[@]}"; do
		LLVM_PGO_PROFILE_URLS+=" gs://chromeos-localmirror/distfiles/llvm-profdata-r${rev}.xz"
	done
}
populate_pgo_profile_urls

SRC_URI="llvm_pgo_use? ( ${LLVM_PGO_PROFILE_URLS} )"

LICENSE="LLVM-exception"
# b/332931474: some tools build against LLVM's unstable interfaces, and need to
# be rebuilt by the CQ when LLVM is upgraded.
# (Do not add new packages that depend on LLVM's unstable interface without
# approval from the toolchain team.)
SLOT="8/${PV}"
KEYWORDS="-* amd64"
IUSE="allow-pgo-mismatch cros_host debug +default-compiler-rt +default-libcxx
	doc +lldb +libedit llvm-asserts +llvm-crt llvm_pgo_generate
	llvm_pgo_use_local ncurses test video_cards_radeon +wrapper_ccache"

# ThinLTO costs a lot of CPU and wall time, and often results in no functional
# changes. Default it to off for 9999, since iteration speed is more important
# there.
#
# PGO often isn't valuable with 9999 ebuilds; disable by default there.
if [[ "${PV}" == "9999" ]]; then
	IUSE+=" thinlto llvm_pgo_use"
else
	IUSE+=" +thinlto +llvm_pgo_use"
fi

# This LLVM is not supported for builds on the target.
REQUIRED_USE="cros_host"

# Disable unittests because they are slow and we test on-device.
RESTRICT+=" test"

# Restrict network sandbox to allow reclient to interact with the RBE instance.
# Do not make any changes that make any other network interactions without
# getting approval from the ChromeOS Build team.
RESTRICT+=" network-sandbox"

COMMON_DEPEND="
	app-arch/xz-utils:0=[${MULTILIB_USEDEP}]
	app-arch/zstd:0=[${MULTILIB_USEDEP}]
	lldb? (
		dev-libs/libxml2:2=[${MULTILIB_USEDEP}]
		libedit? ( dev-libs/libedit:0=[${MULTILIB_USEDEP}] )
		ncurses? ( >=sys-libs/ncurses-5.9-r3:0=[${MULTILIB_USEDEP}] )
	)
	sys-libs/libxcrypt:0=[${MULTILIB_USEDEP}]
	sys-libs/zlib:0=[${MULTILIB_USEDEP}]"
# configparser-3.2 breaks the build (3.3 or none at all are fine)
DEPEND="${COMMON_DEPEND}"
RDEPEND="${COMMON_DEPEND}
	abi_x86_32? ( !<=app-emulation/emul-linux-x86-baselibs-20130224-r2
		!app-emulation/emul-linux-x86-baselibs[-abi_x86_32(-)] )
	lldb? ( ${PYTHON_DEPS} )
	!<=sys-devel/llvm-8.0_pre
	!sys-devel/lld
	!sys-devel/clang"
BDEPEND="
	dev-lang/go
	dev-lang/perl
	lldb? ( dev-lang/swig )
	sys-devel/gnuconfig
	${PYTHON_DEPS}
	$(python_gen_cond_dep '
		dev-python/sphinx[${PYTHON_USEDEP}]
		doc? ( dev-python/recommonmark[${PYTHON_USEDEP}] )
	')
"

# pypy gives me around 1700 unresolved tests due to open file limit
# being exceeded. probably GC does not close them fast enough.
REQUIRED_USE="
	llvm_pgo_generate? ( !llvm_pgo_use !llvm_pgo_use_local )
	llvm_pgo_use? ( !llvm_pgo_use_local )
"

# A list of LLVM distribution components to install. Broadly, these are named
# after either subprojects or individual tools.
# TODO(b/306679955): continue shrinking this to a somewhat-minimal set.
LLVM_DISTRIBUTION_COMPONENTS=(
	# b/306679955: components that are necessary
	builtins
	clang
	clang-format
	clang-resource-headers
	clang-tidy
	clangd
	compiler-rt
	llc
	lld
	llvm-ar
	llvm-as
	llvm-config
	llvm-dwp
	llvm-ifs
	llvm-objcopy
	llvm-objdump
	llvm-ranlib
	llvm-readelf
	llvm-readobj
	llvm-size
	llvm-strings
	llvm-strip

	# b/306679955: components that are almost certainly necessary
	clang-cmake-exports
	cmake-exports
	llvm-dwarfdump
	llvm-cov
	llvm-lib
	llvm-dwarfutil
	dsymutil
	llvm-nm
	llvm-symbolizer
	opt
	sancov
	# TODO: llvm-profgen maybe should be used instead of autofdo?
	llvm-profdata
	llvm-profgen

	# b/319839852: components that would be nice to minimize
	llvm-libraries
)

# Additional components to install if `use lldb`.
LLDB_DISTRIBUTION_COMPONENTS=(
	liblldb
	lldb
	lldb-argdumper
	lldb-instr
	lldb-server
)

get_pgo_profile_name_for_current_revision() {
	local current_rev="$(cros-llvm_get_most_recent_revision)"
	local rev_with_suffix rev
	for rev_with_suffix in "${LLVM_PGO_PROFILE_REVS[@]}"; do
		rev="${rev_with_suffix%%-*}"
		if [[ "${rev}" -eq "${current_rev}" ]]; then
			echo "llvm-profdata-r${rev_with_suffix}"
			return
		fi
	done

	# Subtle convenience feature:
	# If we're in the 9999 ebuild, there may be local commits that are
	# skewing our "most recent revision" calculation forward. Fudge numbers
	# a bit to see if we can find a probably-good-enough profile.
	[[ "${PV}" != "9999" ]] && return

	# Arbitrarily search as many as 50 revisions back. This allows devs to
	# have up to 50 local commits for this to work with.
	local fudged_lower_bound="$((current_rev - 50))"
	for rev_with_suffix in "${LLVM_PGO_PROFILE_REVS[@]}"; do
		rev="${rev_with_suffix%%-*}"
		if [[ "${rev}" -lt "${current_rev}" && "${fudged_lower_bound}" -le "${rev}" ]]; then
			ewarn "No profile found for r${current_rev}; selecting" \
				"profile for r${rev} instead."
			echo "llvm-profdata-r${rev_with_suffix}"
			return
		fi
	done
}

pkg_setup() {
	python-single-r1_pkg_setup
	cros-workon_pkg_setup
}

# Location to source a PGO profile from. Set by src_unpack, used by
# src_configure. Empty if none.
PGO_PROFILE_LOCATION=

src_unpack() {
	cros-workon_src_unpack

	# Optionally unpack a PGO profile, and set PGO_PROFILE_LOCATION for
	# later.
	if use llvm_pgo_use_local; then
		PGO_PROFILE_LOCATION="${FILESDIR}/llvm-local.profdata"
	elif use llvm_pgo_use; then
		local profile_name="$(get_pgo_profile_name_for_current_revision)"
		if [[ -z "${profile_name}" ]]; then
			local complaint=(
				"LLVM: No PGO profile found for current"
				"revision $(cros-llvm_get_most_recent_revision);"
				"cannot apply PGO. Available profiles:"
				"${LLVM_PGO_PROFILE_REVS[*]}."
			)
			# `eerror` instead of `die`ing if allow-pgo-mismatch is
			# set (generally just for LLVM testing).
			if ! use allow-pgo-mismatch; then
				local extra="NOTE: If this is early in an LLVM uprev, including a testing-helper CL in your build will make this nonfatal."
				die "${complaint[*]}" "${extra}"
			fi
			eerror "${complaint[@]}"
		else
			(
				cd "${WORKDIR}" || die
				unpack "${profile_name}.xz"
			)
			PGO_PROFILE_LOCATION="${WORKDIR}/llvm.profdata"
			mv "${WORKDIR}/${profile_name}" "${PGO_PROFILE_LOCATION}" || die
		fi
	fi
}

src_prepare() {
	python_setup

	cros-llvm_ensure_patches_applied

	export CMAKE_USE_DIR="${S}/llvm"
	cmake_src_prepare

	# Add a `/build` subdir to cros-workon's, since cros-workon wants all
	# artifacts to go _into_ the build directory, and multilib wants to
	# create dirs _adjacent_ to the build directory.
	[[ "${PV}" == "9999" ]] && BUILD_DIR="$(cros-workon_get_build_dir)/build"

	# Native libdir is used to hold LLVMgold.so
	# shellcheck disable=SC2034
	NATIVE_LIBDIR=$(get_libdir)
}

multilib_src_configure() {
	export CMAKE_BUILD_TYPE="RelWithDebInfo"

	append-flags -Wno-poison-system-directories

	local enable_asserts=no
	if use debug || use llvm-asserts; then
		enable_asserts=yes
	fi


	local distribution_components=( "${LLVM_DISTRIBUTION_COMPONENTS[@]}" )
	local llvm_projects="llvm;clang;lld;compiler-rt;clang-tools-extra"
	if use lldb; then
		distribution_components+=( "${LLDB_DISTRIBUTION_COMPONENTS[@]}" )
		llvm_projects+=";lldb"
	fi

	local libdir=$(get_libdir)
	local mycmakeargs=(
		"${mycmakeargs[@]}"
		"-DLLVM_ENABLE_PROJECTS=${llvm_projects}"
		"-DLLVM_LIBDIR_SUFFIX=${libdir#lib}"

		"-DLLVM_BUILD_LLVM_DYLIB=ON"
		# Link LLVM statically
		"-DLLVM_LINK_LLVM_DYLIB=OFF"
		"-DBUILD_SHARED_LIBS=OFF"

		# for LLVM breakages specific to BPF (only) contact
		# cros-enterprise-security@google.com for everything else
		# contact OWNERs
		# Please note that RISCV support is *experimental*, so it's not
		# guaranteed to work. If you want to use it, please contact the
		# ChromeOS Toolchain Team.
		"-DLLVM_TARGETS_TO_BUILD=host;X86;ARM;AArch64;NVPTX;BPF;RISCV"

		# Disable IDE support for hacking on _this LLVM build_,
		# which we don't care about, and is incompatible with the
		# LLVM_DISTRIBUTION_COMPONENTS flags below.
		"-DLLVM_ENABLE_IDE=OFF"
		"-DLLVM_DISTRIBUTION_COMPONENTS=$(IFS=';'; echo "${distribution_components[*]}")"

		"-DLLVM_INCLUDE_BENCHMARKS=$(usex test)"
		"-DLLVM_INCLUDE_EXAMPLES=$(usex test)"
		"-DLLVM_INCLUDE_TESTS=$(usex test)"
		"-DLLVM_BUILD_TESTS=$(usex test)"

		"-DLLVM_ENABLE_FFI=NO"
		"-DLLVM_ENABLE_TERMINFO=$(usex ncurses)"
		"-DLLVM_ENABLE_ASSERTIONS=${enable_asserts}"
		"-DLLVM_ENABLE_EH=ON"
		"-DLLVM_ENABLE_RTTI=ON"
		# libxml is only used for llvm-mt, which is a Windows-specific tool.
		"-DLLVM_ENABLE_LIBXML2=OFF"

		"-DLLVM_HOST_TRIPLE=${CHOST}"

		"-DLLVM_BINUTILS_INCDIR=${SYSROOT}/usr/include"

		"-DHAVE_HISTEDIT_H=$(usex libedit)"
		"-DENABLE_LINKER_BUILD_ID=ON"
		"-DCLANG_VENDOR=Chromium OS ${PV}"
		# override default stdlib and rtlib
		"-DCLANG_DEFAULT_CXX_STDLIB=$(usex default-libcxx libc++ "")"
		"-DCLANG_DEFAULT_RTLIB=$(usex default-compiler-rt compiler-rt "")"
		"-DCLANG_DEFAULT_LINKER=lld"

		# Link against libcxx
		"-DLLVM_ENABLE_LIBCXX=Yes"
		# Use lld to link llvm
		"-DLLVM_USE_LINKER=lld"

		# crbug/855759
		"-DCOMPILER_RT_BUILD_CRT=$(usex llvm-crt)"

		"-DCMAKE_POSITION_INDEPENDENT_CODE=ON"
		"-DCLANG_DEFAULT_UNWINDLIB=libgcc"
		"-DCLANG_DEFAULT_PIE_ON_LINUX=ON"

		# workaround for crbug/1198796
		"-DCLANG_TOOLING_BUILD_AST_INTROSPECTION=OFF"
		# No one uses plugins; turning them off makes LLVM marginally
		# smaller & faster.
		"-DCLANG_PLUGIN_SUPPORT=OFF"

		# By default do not enable PGO for compiler-rt
		"-DCOMPILER_RT_ENABLE_PGO=OFF"

		# compiler-rt needs libc++ sources to be specified to build
		# an internal copy for libfuzzer, can be removed if libc++
		# is built inside llvm ebuild.
		"-DCOMPILER_RT_LIBCXXABI_PATH=${S}/libcxxabi"
		"-DCOMPILER_RT_LIBCXX_PATH=${S}/libcxx"
		"-DCOMPILER_RT_BUILTINS_HIDE_SYMBOLS=OFF"

		# b/200831212: Disable per runtime install dirs.
		"-DLLVM_ENABLE_PER_TARGET_RUNTIME_DIR=OFF"

		# b/202073091: Disable Lua.
		"-DLLDB_ENABLE_LUA=OFF"

		# b/204220308: Disable ORC since we are not using it.
		"-DCOMPILER_RT_BUILD_ORC=OFF"

		# b/241569725: Explicitly set Python version so that it
		# chooses the version we're currently supporting
		"-DPython3_EXECUTABLE=${PYTHON}"

		# We don't use ocaml bindings, so don't worry about ocamlfind.
		"-DOCAMLFIND=NO"
	)

	# Inform the LLVM ebuild that it's cross-compiling if ${ROOT} is set.
	# Otherwise, it will build host tools against target libraries/headers.
	# This is generally incorrect, and specifically breaks the build e.g.,
	# during glibc upgrades.
	[[ -n "${ROOT}" ]] && mycmakeargs+=(
		# Set this according to LLVM's cross-compiling instructions:
		# https://llvm.org/docs/HowToCrossCompileLLVM.html. While a
		# reasonable default is selected if this is set, setting it
		# explicitly causes CMake to set other variables for a
		# cross-compile. LLVM recognizes these specially.
		"-DCMAKE_SYSTEM_NAME=Linux"
	)

	if use lldb; then
		# lldb refuses to detect these on its own if `-DCMAKE_SYSTEM_NAME` is
		# set. Use the same mechanism as cmake to force these to the right
		# values, since we're always 'cross-compiling' to the same system as the
		# host.
		local arg result
		for arg in LLDB_PYTHON_EXE_RELATIVE_PATH LLDB_PYTHON_RELATIVE_PATH LLDB_PYTHON_EXT_SUFFIX; do
			result="$("${PYTHON}" "${S}/lldb/bindings/python/get-python-config.py" "${arg}")" || die
			mycmakeargs+=( "-D${arg}=${result}" )
		done
	fi

	# The standalone toolchain may be run at places not supporting smallPIE,
	# disabling it for lld.
	# Pass -fuse-ld=lld to make cmake happy.
	append-ldflags "-fuse-ld=lld -Wl,--pack-dyn-relocs=none"

	if use thinlto; then
		mycmakeargs+=(
			"-DLLVM_ENABLE_LTO=thin"
			# b/228090090: LLVM defaults to 2 parallel link
			# jobs with ThinLTO enabled. Bumping this to 4
			# speeds `emerge llvm` up on dev machines by
			# 1.25x, and saves >10mins per LLVM build on the
			# SDK builder.
			"-DLLVM_PARALLEL_LINK_JOBS=4"
			# b/298428344: Disable ThinLTO's cache, as it makes our
			# LLVM builds nondeterministic. The observed
			# nondeterminism is functionally irrelevant (literally
			# just an extra symbol in .symtab), but the bazel effort
			# seeks bit determinism.
			"-DLLVM_THINLTO_CACHE_PATH="
		)
		# b/227370760: Instr limits above 30 don't seem to help
		# our performance (and might hurt in some cases). They
		# also make builds take meaningfully longer, and add
		# tens of MB to our SDK size. We tune this back in other
		# large ThinLTO users (e.g., Chrome), so do it here,
		# too.
		append-ldflags "-Wl,-mllvm,-import-instr-limit=30"

		# b/314998647: For faster builds, default to --thinlto-O0,
		# except on perf-critical binaries
		append-ldflags "-Wl,--lto-O0"
		mycmakeargs+=(
			"-DLLVM_clang_LINKER_FLAGS=-Wl,--lto-O3"
			"-DLLVM_lld_LINKER_FLAGS=-Wl,--lto-O3"
		)
	fi

	if use llvm_pgo_generate; then
		# Our benchmarks run out of static counters; allow them
		# to be dynamically allocated (which adds overhead, but
		# increases precision).
		append-flags -mllvm -vp-static-alloc=false
		mycmakeargs+=(
			"-DLLVM_BUILD_INSTRUMENTED=IR"
		)
	elif [[ -n "${PGO_PROFILE_LOCATION}" ]]; then
		mycmakeargs+=(
			"-DLLVM_PROFDATA_FILE=${PGO_PROFILE_LOCATION}"
		)

		# Disable warning about profile not matching. These are emitted
		# when PGO profiles don't apply to functions, and are a signal
		# of code 'drifting' & the PGO profile degrading. Some
		# degredation is expected as a normal part of patching LLVM.
		append-flags "-Wno-backend-plugin"
	fi

	if cros-remoteexec_use_remoteexec; then
		export RBE_inputs="${PGO_PROFILE_LOCATION}"
		mycmakeargs+=(
			-DCMAKE_C_COMPILER_LAUNCHER="${FILESDIR}/reclient_compiler_launcher.sh"
			-DCMAKE_CXX_COMPILER_LAUNCHER="${FILESDIR}/reclient_compiler_launcher.sh"
		)
	elif [[ -z "${CCACHE_DISABLE:-}" ]]; then
		# If ccache is enabled, use it as a compiler launcher. Chances
		# are that folks are iterating on LLVM, and ccache speeds that
		# up substantially.
		mycmakeargs+=(
			"-DCMAKE_C_COMPILER_LAUNCHER=ccache"
			"-DCMAKE_CXX_COMPILER_LAUNCHER=ccache"
		)
	fi

	if multilib_is_native_abi; then
		mycmakeargs+=(
			"-DLLVM_BUILD_DOCS=$(usex doc)"
			"-DLLVM_ENABLE_SPHINX=$(usex doc)"
			"-DLLVM_ENABLE_DOXYGEN=OFF"
			"-DLLVM_INSTALL_UTILS=ON"
		)
	fi

	if [[ "${enable_asserts}" == no ]]; then
		append-cppflags -DNDEBUG
	fi

	cmake_src_configure
}

multilib_src_compile() {
	cros-remoteexec_initialize

	# ...If folks are iterating on LLVM with `emerge`, every successful
	# `emerge` invocation will replace their old compiler. Since doing so
	# updates mtimes, we should be focusing on the _content_ of the
	# compiler, rather than the mtime.
	#
	# Note that this is in `src_compile`, since it won't apply if we do it
	# in `src_configure`.
	export CCACHE_COMPILERCHECK=content

	# N.B., `libclang.so` is needed here since there's no associated
	# component that isn't `clang-libraries`. We don't want to install all
	# Clang static libs.
	cmake_src_compile distribution libclang.so

	pax-mark m "${BUILD_DIR}"/bin/llvm-rtdyld
	pax-mark m "${BUILD_DIR}"/bin/lli
	pax-mark m "${BUILD_DIR}"/bin/lli-child-target

	if use test; then
		pax-mark m "${BUILD_DIR}"/unittests/ExecutionEngine/Orc/OrcJITTests
		pax-mark m "${BUILD_DIR}"/unittests/ExecutionEngine/MCJIT/MCJITTests
		pax-mark m "${BUILD_DIR}"/unittests/Support/SupportTests
	fi

	cros-remoteexec_shutdown
}

multilib_src_test() {
	# respect TMPDIR!
	local -x LIT_PRESERVES_TMP=1
	cmake_src_test
}

src_install() {
	cros-remoteexec_initialize

	local MULTILIB_CHOST_TOOLS=(
		/usr/bin/llvm-config
	)

	local MULTILIB_WRAPPED_HEADERS=(
		/usr/include/llvm/Config/config.h
		/usr/include/llvm/Config/llvm-config.h
	)

	multilib-minimal_src_install

	cros-remoteexec_shutdown
}

# This computes the toolchain SHA that gets baked into our compiler_wrapper
# binaries. This SHA is potentially nice for uniquely identifying a toolchain,
# but the most critical purpose that it serves is making the SHA for our
# compiler_wrapper change with the toolchain that it was installed with. If any
# of these binaries get modified and we fail to update compiler_wrapper, tools
# like ccache, ninja, etc might not expect the updates without extra work (thus,
# cached object files might not be considered stale across compiler updates).
#
# Rather than playing whack-a-mole with ways to inform each build system that
# our compiler has changed (despite its hash remaining identical), we simply
# modify its hash when any of the binaries it calls may change.
compute_toolchain_sha() {
	local toolchain_sha_binaries=(
		"bin/clang"
		"bin/clang-tidy"
		"bin/ld.lld"
	)

	# Compute the SHA sums for each of these in parallel, then hash the
	# result. This hash represents all of the (non-header, non-library)
	# dependencies that Clang has.
	#
	# This is expected to be called in a subshell, so don't try to restore
	# pipefail.
	set -o pipefail
	echo "${toolchain_sha_binaries[*]}" |
		xargs -L1 -P8 sha256sum |
		sort -k2 |
		sha256sum - |
		awk '{print $1}'
}

multilib_src_install() {
	local cmake_targets=(
		install-distribution
		install-llvm-headers
		install-clang-headers
		install-compiler-rt-headers
		install-clang-tidy-headers
	)

	use lldb && cmake_targets+=(
		install-lldb-headers
		install-lldb-python-scripts
	)

	DESTDIR="${D}" cmake_build "${cmake_targets[@]}"

	# Install libclang. No distribution component, so we need to handle it
	# manually.
	einfo "PWD is ${PWD}"
	insinto "/usr/lib64"
	dolib.so lib64/libclang*so*

	# These files are required for sanitizers to function properly, and are
	# built by the `compiler-rt` target. For some reason, they're not placed
	# in a CMake `install` component. Use `find` not because multiple matches
	# are expected, but because the dir contains the LLVM major version,
	# which changes over time. Also shellcheck complains about `ls`, so don't
	# use that.
	#
	# NOTE: There's a bug (b/476469740#comment2) where it _does_ have multiple
	# major versions. Sort and choose the latest one.
	local find_result
	find_result="$(find lib64/clang -mindepth 1 -maxdepth 1 -printf '%f\n')"
	local clang_version_dir
	clang_version_dir="$(printf "%s\n" "${find_result}" | sort -Vr | head -n1)"
	local clang_syms_dir="lib64/clang/${clang_version_dir}/lib/linux"
	insinto "/usr/${clang_syms_dir}"
	doins "${clang_syms_dir}/"*.syms

	(cd "${S}" && einstalldocs)

	local wrapper_script=clang_host_wrapper

	local toolchain_sha
	toolchain_sha="$(compute_toolchain_sha)" || die
	einfo "Computed toolchain SHA = ${toolchain_sha}"

	local common_wrapper_flags=(
		"--version=toolchain_sha_${toolchain_sha}"
		"--llvm_revision=$(cros-llvm_get_most_recent_revision)"
	)

	GO111MODULE=off "${FILESDIR}/compiler_wrapper/build.py" --config=cros.host --use_ccache=false \
		--output_file="${D}/usr/bin/${wrapper_script}" \
		"${common_wrapper_flags[@]}" || die

	newbin "${D}/usr/bin/clang-tidy" "clang-tidy"
	dobin "${FILESDIR}/clang_cc_wrapper"
	exeinto "/usr/bin"
	dosym "${wrapper_script}" "/usr/bin/${CHOST}-clang"
	dosym "${wrapper_script}" "/usr/bin/${CHOST}-clang++"
	newexe "${FILESDIR}/ldwrapper_lld.host" "${CHOST}-ld.lld"

	# llvm-strip is a symlink to llvm-objcopy and distinguished by a argv[0] check.
	# When creating standalone toolchain, argv[0] information is lost and causes
	# llvm-strip invocations to be treated as llvm-objcopy breaking builds
	# (crbug.com/1151787). Handle this by making llvm-strip a full binary.
	if [[ -L "${D}/usr/bin/llvm-strip" ]]; then
		rm "${D}/usr/bin/llvm-strip" || die
		newbin "${D}/usr/bin/llvm-objcopy" "llvm-strip"
	fi

	# Build and install cross-compiler wrappers for supported ABIs.
	# ccache wrapper is used in chroot and non-ccache wrapper is used
	# in standalone SDK.
	local ccache_suffixes=(noccache ccache)
	local ccache_option_values=(false true)
	for ccache_index in {0,1}; do
		local ccache_suffix="${ccache_suffixes[${ccache_index}]}"
		local ccache_option="${ccache_option_values[${ccache_index}]}"
		# Build hardened wrapper written in golang.
		GO111MODULE=off "${FILESDIR}/compiler_wrapper/build.py" --config="cros.hardened" \
			--use_ccache="${ccache_option}" \
			--output_file="${D}/usr/bin/sysroot_wrapper.hardened.${ccache_suffix}" \
			"${common_wrapper_flags[@]}" || die

		# Build non-hardened wrapper written in golang.
		GO111MODULE=off "${FILESDIR}/compiler_wrapper/build.py" --config="cros.nonhardened" \
			--use_ccache="${ccache_option}" \
			--output_file="${D}/usr/bin/sysroot_wrapper.${ccache_suffix}" \
			"${common_wrapper_flags[@]}" || die

		# Try to catch a case where the sysroot_wrapper is empty. Seems to be a very
		# tricky race condition. Tracked at b/203821449.
		local message_tail="had size zero or didn't exist after build. Please see b/203821449."
		if [[ ! -s "${D}/usr/bin/sysroot_wrapper.${ccache_suffix}" ]]; then
			die "${D}/usr/bin/sysroot_wrapper.${ccache_suffix} ${message_tail}"
		fi

		if [[ ! -s "${D}/usr/bin/sysroot_wrapper.hardened.${ccache_suffix}" ]]; then
			die "${D}/usr/bin/sysroot_wrapper.hardened.${ccache_suffix} ${message_tail}"
		fi
	done

	local cros_hardened_targets=(
		# TODO(b/266132652): Add new llvm-libc baremetal targets
		"aarch64-cros-linux-gnu"
		"armv7a-cros-linux-gnueabihf"
		"i686-cros-linux-gnu"
		"x86_64-cros-linux-gnu"
	)
	local cros_nonhardened_targets=(
		"arm-none-eabi"
		"armv7m-cros-eabi"
		"riscv32-cros-elf"
	)

	local use_ccache_index
	use_ccache_index="$(usex wrapper_ccache 1 0)"
	local sysroot_wrapper_suffix="${ccache_suffixes[${use_ccache_index}]}"

	local target
	for target in "${cros_hardened_targets[@]}"; do
		dosym "sysroot_wrapper.hardened.${sysroot_wrapper_suffix}" "/usr/bin/${target}-clang"
		dosym "sysroot_wrapper.hardened.${sysroot_wrapper_suffix}" "/usr/bin/${target}-clang++"
	done
	for target in "${cros_nonhardened_targets[@]}"; do
		dosym "sysroot_wrapper.${sysroot_wrapper_suffix}" "/usr/bin/${target}-clang"
		dosym "sysroot_wrapper.${sysroot_wrapper_suffix}" "/usr/bin/${target}-clang++"
	done

	# Install a symlink bpf-clang which point to clang.
	# Running through symlink will make clang default to bpf target.
	dosym "clang" "/usr/bin/bpf-clang"
}

multilib_src_install_all() {
	insinto /usr/share/vim/vimfiles
	doins -r llvm/utils/vim/*/.
	# some users may find it useful
	dodoc llvm/utils/vim/vimrc
	dobin "${S}/compiler-rt/lib/asan/scripts/asan_symbolize.py"
}

pkg_postinst() {
	if has_version "dev-util/ccache" ; then
		#add ccache links as clang might get installed after ccache
		"${EROOT}"/usr/bin/ccache-config --install-links
	fi
}

pkg_postrm() {
	if has_version "dev-util/ccache" && [[ -z ${REPLACED_BY_VERSION} ]]; then
		# --remove-links would remove all links, --install-links updates them
		"${EROOT}"/usr/bin/ccache-config --install-links
	fi
}
