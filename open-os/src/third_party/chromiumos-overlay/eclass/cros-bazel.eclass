# Copyright 2019 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

# @ECLASS: cros-bazel.eclass
# @MAINTAINER:
# Michael Martis <martis@chromium.org>
# @DESCRIPTION:
# A utility eclass for the Bazel build system. Based on the now-abandoned
# upstream Bazel eclass, plus ChromiumOS-specific addition like cross
# compilation support.

if [[ -z ${_CROS_BAZEL_ECLASS} ]]; then

# Check for EAPI 7+.
case "${EAPI:-0}" in
[0123456]) die "Unsupported EAPI=${EAPI:-0} (too old) for ${ECLASS}";;
esac

# Don't depend on the system VM since it could be pointing to Java 8.
export GENTOO_VM=openjdk-bin-11

inherit multiprocessing cros-toolchain-funcs

BDEPEND=">=dev-util/bazel-0.20"

# @ECLASS-VARIABLE: BAZEL_BINARY
# @DESCRIPTION:
# The program to invoke for bazel. Defaults to `bazel`. Useful if you have
# multiple bazel installations on your machine that differ in version suffix,
# e.g., `bazel-5`, `bazel-6`.
: "${BAZEL_BINARY:=bazel}"

# @ECLASS-VARIABLE: BAZEL_BAZELRC
# @DESCRIPTION:
# The location of the resource file used to provide Portage's build
# configuration details to Bazel. Must be kept in sync with the Bazel eclass.
BAZEL_BAZELRC="${T}/bazelrc"

# @ECLASS-VARIABLE: BAZEL_CC_BAZELRC
# @INTERNAL
# @DESCRIPTION:
# The location of the resource file specifying build configuration details for
# cross compilation (if setup). Is 'sourced' by BAZEL_BAZELRC.
BAZEL_CC_BAZELRC="${T}/cc_bazelrc"

# @ECLASS-VARIABLE: BAZEL_PORTAGE_PACKAGE_DIR
# @INTERNAL
# @DESCRIPTION:
# The directory used to store generated configuration targets (e.g. toolchain
# targets for cross compilation).
BAZEL_PORTAGE_PACKAGE_DIR="${T}/portage_packages/"

# @ECLASS-VARIABLE: BAZEL_CC_CONFIG_DIR
# @INTERNAL
# @DESCRIPTION:
# The directory (relative to BAZEL_PORTAGE_PACKAGE_DIR) in which "host" and
# "target" toolchain targets are generated for cross compilation.
BAZEL_CC_CONFIG_DIR="ebazel_cc_config"

# @ECLASS-VARIABLE: BAZEL_CC_BUILD
# @INTERNAL
# @DESCRIPTION:
# A template (with Bash-style variable placeholders) used to populate build
# files for both the "host" and "target" toolchain targets.
# shellcheck disable=SC2016
BAZEL_CC_BUILD='package(default_visibility = ["//visibility:public"])

filegroup(name = "empty")

amd64_constraints = [
	"@platforms//cpu:x86_64",
	"@platforms//os:linux",
]

k8_constraints = amd64_constraints

arm_constraints = [
	"@platforms//cpu:arm",
	"@platforms//os:linux",
]

armv7a_constraints = arm_constraints

aarch64_constraints = [
	"@platforms//cpu:aarch64",
	"@platforms//os:linux",
]

arm64_constraints = aarch64_constraints

platform(
	name = "amd64_platform",
	constraint_values = amd64_constraints,
)

platform(
	name = "k8_platform",
	constraint_values = k8_constraints,
)

platform(
	name = "arm_platform",
	constraint_values = arm_constraints,
)

platform(
	name = "armv7a_platform",
	constraint_values = armv7a_constraints,
)

platform(
	name = "aarch64_platform",
	constraint_values = aarch64_constraints,
)

platform(
	name = "arm64_platform",
	constraint_values = arm64_constraints,
)

cc_toolchain_suite(
	name = "toolchain",
	toolchains = {
		"amd64|local": "portage_toolchain",
		"k8|local": "portage_toolchain",
		"arm|local": "portage_toolchain",
		"armv7a|local": "portage_toolchain",
		"aarch64|local": "portage_toolchain",
		"arm64|local": "portage_toolchain",
	},
)

cc_toolchain(
	name = "portage_toolchain",
	toolchain_identifier = "portage-toolchain",
	toolchain_config = ":portage_toolchain_config",
	all_files = ":empty",
	compiler_files = ":empty",
	dwp_files = ":empty",
	linker_files = ":empty",
	objcopy_files = ":empty",
	strip_files = ":empty",
	supports_param_files = 0,
)

toolchain(
	name = "cc-toolchain-${cpu_str}",
	# compilation execution is always on the host, hence amd64
	exec_compatible_with = amd64_constraints,
	target_compatible_with = ${cpu_str}_constraints,
	toolchain = ":portage_toolchain",
	toolchain_type = "@bazel_tools//tools/cpp:toolchain_type",
)

load(":cc_toolchain_config.bzl", "cc_toolchain_config")
cc_toolchain_config(name = "portage_toolchain_config")
'

# @ECLASS-VARIABLE: BAZEL_CC_TOOLCHAIN_CONFIG
# @INTERNAL
# @DESCRIPTION:
# Skylark implementation of the cc toolchain, using Bash-style variables
# to populate the build file.
# shellcheck disable=SC2016
BAZEL_CC_TOOLCHAIN_CONFIG='
load("@bazel_tools//tools/build_defs/cc:action_names.bzl", "ACTION_NAMES")
load(
  "@bazel_tools//tools/cpp:cc_toolchain_config_lib.bzl",
  "feature",
  "flag_group",
  "flag_set",
  "tool_path",
)

features = [
  feature(name="supports_pic", enabled=True),
  feature(
    name="determinism",
    flag_sets = [
      flag_set(
        actions = [ACTION_NAMES.c_compile, ACTION_NAMES.cpp_compile],
        flag_groups = [
          flag_group(
            flags = [
              # Make C++ compilation deterministic. Use linkstamping instead of these
              # compiler symbols.
              "-Wno-builtin-macro-redefined",
              "-D__DATE__=\"redacted\"",
              "-D__TIMESTAMP__=\"redacted\"",
              "-D__TIME__=\"redacted\"",
            ]
          )
        ]
      ),
    ]
  ),
  feature(
    name="hardening",
    flag_sets = [
      flag_set(
        actions = [ACTION_NAMES.c_compile, ACTION_NAMES.cpp_compile],
        flag_groups = [
          flag_group(
            flags = [
              # Conservative choice; -D_FORTIFY_SOURCE=2 may be unsafe in some cases.
              # We need to undef it before redefining it as some distributions now
              # have it enabled by default.
              "-U_FORTIFY_SOURCE",
              "-D_FORTIFY_SOURCE=1",
              "-fstack-protector",
            ]
          )
        ]
      ),
      flag_set(
        actions = [
          ACTION_NAMES.cpp_link_dynamic_library,
          ACTION_NAMES.cpp_link_nodeps_dynamic_library,
        ],
        flag_groups = [flag_group(flags = ["-Wl,-z,relro,-z,now"])]
      ),
      flag_set(
        actions = [
          ACTION_NAMES.cpp_link_executable,
        ],
        flag_groups = [flag_group(flags = ["-pie", "-Wl,-z,relro,-z,now"])]
      ),
    ]
  ),
  feature(
    name="warnings",
    flag_sets = [
      flag_set(
        actions = [ACTION_NAMES.c_compile, ACTION_NAMES.cpp_compile],
        flag_groups = [
          flag_group(
            flags = [
              # Mirrors logic from crrev.com/c/6388817 (b/403315166).
              # Some tool configurations in packages like sci-libs/tensorflow
              # ignore host _and_ target CFLAGS from portage, but not these.
              "-D_CROSTC_ADD_IMPLICIT_CFLAGS_FOR='"${CATEGORY}/${PN}"'",
              # All warnings are enabled. Maybe enable -Werror as well?
              "-Wall",
              # Add another warning that is not part of -Wall.
              "-Wunused-but-set-parameter",
              # But disable some that are problematic.
              "-Wno-free-nonheap-object" # has false positives
            ]
          )
        ]
      ),
    ]
  ),
  feature(
    name="no-canonical-prefixes",
    flag_sets = [
      flag_set(
        actions = [
          ACTION_NAMES.assemble,
          ACTION_NAMES.c_compile,
          ACTION_NAMES.cpp_compile,
          ACTION_NAMES.cpp_link_dynamic_library,
          ACTION_NAMES.cpp_link_nodeps_dynamic_library,
          ACTION_NAMES.cpp_link_executable,
          ACTION_NAMES.preprocess_assemble,
        ],
        flag_groups = [flag_group(flags = ["-no-canonical-prefixes"])]
      ),
    ]
  ),
  feature(
    name="linker-bin-path",
    flag_sets = [
      flag_set(
        actions = [
          ACTION_NAMES.cpp_link_dynamic_library,
          ACTION_NAMES.cpp_link_nodeps_dynamic_library,
          ACTION_NAMES.cpp_link_executable,
        ],
        flag_groups = [flag_group(flags = ["-B/usr/bin/"])]
      ),
    ]
  ),
  feature(
    name="disable-assertions",
    flag_sets = [
      flag_set(
        actions = [ACTION_NAMES.c_compile, ACTION_NAMES.cpp_compile],
        flag_groups = [flag_group(flags = ["-DNDEBUG"])]
      ),
    ]
  ),
  feature(
    name="common",
    implies=[
      "determinism",
      "hardening",
      "warnings",
      "no-canonical-prefixes",
      "linker-bin-path"
    ],
  ),
  feature(
    name="opt",
    implies=["common"],
    flag_sets = [
      flag_set(
        actions = [ACTION_NAMES.c_compile, ACTION_NAMES.cpp_compile],
        flag_groups = [
          flag_group(
            flags = ["-g0", "-O2", "-ffunction-sections", "-fdata-sections"]
          )
        ]
      ),
      flag_set(
        actions = [
          ACTION_NAMES.cpp_link_dynamic_library,
          ACTION_NAMES.cpp_link_nodeps_dynamic_library,
          ACTION_NAMES.cpp_link_executable,
        ],
        flag_groups = [
          flag_group(
            flags = ["-Wl,--gc-sections"]
          )
        ]
      )
    ]
  ),
  feature(
    name="fastbuild",
    implies=["common"],
  ),
  feature(
    name="dbg",
    implies=["common"],
    flag_sets = [
      flag_set(
        actions = [ACTION_NAMES.c_compile, ACTION_NAMES.cpp_compile],
        flag_groups = [
          flag_group(
            flags = ["-g"]
          )
        ]
      )
    ]
  ),
]

def _impl(ctx):
  tool_paths = [
    tool_path(name = "gcc", path = "${env_cc}"),
    tool_path(name = "ar", path = "${env_ar}"),
    tool_path(name = "compat-ld", path = "${env_ld}"),
    tool_path(name = "cpp", path = "${env_cpp}"),
    tool_path(name = "dwp", path = "${env_dwp}"),
    tool_path(name = "ld", path = "${env_ld}"),
    tool_path(name = "nm", path = "${env_nm}"),
    tool_path(name = "objcopy", path = "${env_objcopy}"),
    tool_path(name = "objdump", path = "${env_objdump}"),
    tool_path(name = "strip", path = "${env_strip}"),
  ]

  return cc_common.create_cc_toolchain_config_info(
    ctx = ctx,
    features = features,
    cxx_builtin_include_directories = [
      ${builtin_include_dirs}
    ],
    builtin_sysroot="${env_sysroot}",
    toolchain_identifier = "portage-toolchain",
    host_system_name = "local",
    target_system_name = "local",
    target_cpu = "${cpu_str}",
    target_libc = "local",
    compiler = "local",
    abi_version = "local",
    abi_libc_version = "local",
    tool_paths = tool_paths,
  )

cc_toolchain_config = rule(
  implementation = _impl,
  attrs = {},
  provides = [CcToolchainConfigInfo],
)
'

# @FUNCTION: bazel_get_flags
# @DESCRIPTION:
# Obtain and print the bazel flags for target and host *FLAGS.
#
# To add more flags to this, append the flags to the
# appropriate variable before calling this function
bazel_get_flags() {
	local i fs=()
	for i in ${CFLAGS}; do
		fs+=( "--conlyopt=${i}" )
	done
	# ignore missing BUILD_*FLAGS definition lint
	# shellcheck disable=2154
	for i in ${BUILD_CFLAGS}; do
		fs+=( "--host_conlyopt=${i}" )
	done
	for i in ${CXXFLAGS}; do
		fs+=( "--cxxopt=${i}" )
	done
	# shellcheck disable=2154
	for i in ${BUILD_CXXFLAGS}; do
		fs+=( "--host_cxxopt=${i}" )
	done
	for i in ${CPPFLAGS}; do
		fs+=( "--conlyopt=${i}" "--cxxopt=${i}" )
	done
	# shellcheck disable=2154
	for i in ${BUILD_CPPFLAGS}; do
		fs+=( "--host_conlyopt=${i}" "--host_cxxopt=${i}" )
	done
	for i in ${LDFLAGS}; do
		fs+=( "--linkopt=${i}" )
	done
	# shellcheck disable=2154
	for i in ${BUILD_LDFLAGS}; do
		fs+=( "--host_linkopt=${i}" )
	done

	# Temporarily disable sanitizer blacklist until upstream issue
	# https://github.com/bazelbuild/bazel/issues/10561 is fixed.
	fs+=( "--copt=-fno-sanitize-blacklist" )

	echo "${fs[*]}"
}

# @FUNCTION: bazel_setup_bazelrc
# @DESCRIPTION:
# Creates the bazelrc with common options that will be passed
# to bazel. This will be called by ebazel automatically so
# does not need to be called from the ebuild.
bazel_setup_bazelrc() {
	if [[ -f "${T}/bazelrc" ]]; then
		return
	fi

	# F: fopen_wr
	# P: /proc/self/setgroups
	# Even with standalone enabled, the Bazel sandbox binary is run for feature test:
	# https://github.com/bazelbuild/bazel/blob/7b091c1397a82258e26ab5336df6c8dae1d97384/src/main/java/com/google/devtools/build/lib/sandbox/LinuxSandboxedSpawnRunner.java#L61
	# https://github.com/bazelbuild/bazel/blob/76555482873ffcf1d32fb40106f89231b37f850a/src/main/tools/linux-sandbox-pid1.cc#L113
	addpredict /proc

	mkdir -p "${T}/bazel-cache" || die
	mkdir -p "${T}/bazel-distdir" || die

	cat > "${T}/bazelrc" <<-EOF || die
		startup --batch

		# dont strip HOME, portage sets a temp per-package dir
		build --action_env HOME
		# similarly for CROS_ARTIFACTS_TMP_DIR, some actions need this (e.g.,
		# compiler crash dumps: b/417179090); it ends up in a dir similar to the
		# package-local HOME.
		build --action_env CROS_ARTIFACTS_TMP_DIR

		# make bazel respect MAKEOPTS
		build --jobs=$(makeopts_jobs)
		build --compilation_mode=opt --host_compilation_mode=opt

		# FLAGS
		build $(bazel_get_flags)

		# Use standalone strategy to deactivate the bazel sandbox, since it
		# conflicts with FEATURES=sandbox.
		build --spawn_strategy=standalone --genrule_strategy=standalone
		test --spawn_strategy=standalone --genrule_strategy=standalone

		build --strip=never
		build --verbose_failures --noshow_loading_progress
		test --verbose_test_summary --verbose_failures --noshow_loading_progress

		# make bazel only fetch distfiles from the cache
		fetch --repository_cache="${T}/bazel-cache/" --distdir="${T}/bazel-distdir/"
		build --repository_cache="${T}/bazel-cache/" --distdir="${T}/bazel-distdir/"

		build --define=PREFIX=${EPREFIX%/}/usr
		build --define=LIBDIR=\$(PREFIX)/$(get_libdir)
		build --define=INCLUDEDIR=\$(PREFIX)/include
		EOF
}

# @FUNCTION: ebazel
# @USAGE: [<args>...]
# @DESCRIPTION:
# Run bazel with the bazelrc and output_base.
#
# output_base will be specific to $BUILD_DIR (if unset, $S).
# bazel_setup_bazelrc will be called and the created bazelrc
# will be passed to bazel.
#
# Will automatically die if bazel does not exit cleanly.
ebazel() {
	bazel_setup_bazelrc

	# Use different build folders for each multibuild variant.
	local output_base="${BUILD_DIR:-${S}}"
	output_base="${output_base%/}-bazel-base"
	mkdir -p "${output_base}" || die

	set -- "${BAZEL_BINARY}" --bazelrc="${T}/bazelrc" --output_base="${output_base}" "${@}"
	echo "${*}" >&2
	"${@}" || die "ebazel failed"
}

# @FUNCTION: bazel_load_distfiles
# @USAGE: <distfiles>...
# @DESCRIPTION:
# Populate the bazel distdir to fetch from since it cannot use
# the network. Bazel looks in distdir but will only look for the
# original filename, not the possibly renamed one that portage
# downloaded. If the line has -> we to rename it back. This also
# handles use-conditionals that SRC_URI does.
#
# Example:
# @CODE
# bazel_external_uris="http://a/file-2.0.tgz
#     python? ( http://b/1.0.tgz -> foo-1.0.tgz )"
# SRC_URI="http://c/${PV}.tgz
#     ${bazel_external_uris}"
#
# src_unpack() {
#     unpack ${PV}.tgz
#     bazel_load_distfiles "${bazel_external_uris}"
# }
# @CODE
bazel_load_distfiles() {
	local file=""
	local rename=0

	[[ $# -gt 0 ]] || die "Missing args"
	mkdir -p "${T}/bazel-distdir" || die

	# shellcheck disable=SC2068
	for word in ${@}
	do
		if [[ "${word}" == "->" ]]; then
			# next word is a dest filename
			rename=1
		elif [[ "${word}" == ")" ]]; then
			# close conditional block
			continue
		elif [[ "${word}" == "(" ]]; then
			# open conditional block
			continue
		elif [[ "${word}" == ?(\!)[A-Za-z0-9]*([A-Za-z0-9+_@-])\? ]]; then
			# use-conditional block
			# USE-flags can contain [A-Za-z0-9+_@-], and start with alphanum
			# https://dev.gentoo.org/~ulm/pms/head/pms.html#x1-200003.1.4
			# ?(\!) matches zero-or-one !'s
			# *(...) zero-or-more characters
			# ends with a ?
			continue
		elif [[ ${rename} -eq 1 ]]; then
			# Make sure the distfile is used
			if [[ "${A}" == *"${word}"* ]]; then
				echo "Copying ${file} to bazel distdir as ${word}"
				ln -s "${DISTDIR}/${word}" "${T}/bazel-distdir/${file}" || die
			fi
			rename=0
			file=""
		else
			# another URL, current one may or may not be a rename
			# if there was a previous one, its not renamed so copy it now
			if [[ -n "${file}" && "${A}" == *"${file}"* ]]; then
				echo "Copying ${file} to bazel distdir"
				ln -s "${DISTDIR}/${file}" "${T}/bazel-distdir/${file}" || die
			fi
			# save the current URL, later we will find out if its a rename or not.
			file="${word##*/}"
		fi
	done

	# handle last file
	if [[ -n "${file}" ]]; then
		echo "Copying ${file} to bazel distdir"
		ln -s "${DISTDIR}/${file}" "${T}/bazel-distdir/${file}" || die
	fi
}

# @FUNCTION: bazel_get_builtin_include_dirs
# @USAGE: <compiler binary>
# @RETURN:
# A list of the directories that are searched by default on invocation of the
# given compiler's preprocessor. These directories are normalized (e.g.
# parsing "..") and formatted as a python list of strings.
# @MAINTAINER:
# Michael Martis <martis@chromium.org>
# @INTERNAL
bazel_get_builtin_include_dirs() {
	# Constants that demarcate default include dir information.
	local match_head="#include <...> search starts here:"
	local match_foot="End of search list."

	local comp="${1}"

	# Get preprocessor output (which contains searched include dirs).
	local preproc_output
	preproc_output="$("${comp}" -E -xc++ -Wp,-v - 2>&1 <<< "int main() { return 0; }" || die)"

	# Keep only the include dirs (which are between two known markers).
	local include_dirs
	include_dirs="$(sed "1,/${match_head}/d;/${match_foot}/,\$d" <<< "${preproc_output}" || die)"

	# For each include dir...
	while read -r include_dir; do
		# Normalize (e.g. process '..' sequences in) the path.
		local norm_dir
		# shellcheck disable=SC2015
		norm_dir="$(cd "${include_dir}" && pwd || die)"

		# Print the normalized path as a proto field.
		echo "\"${norm_dir}\","
	done <<< "${include_dirs}"
}

# @FUNCTION: bazel_populate_crosstool_target
# @USAGE: <sysroot> <prefix> <cpu string> <output directory>
# @MAINTAINER:
# Michael Martis <martis@chromium.org>
# @INTERNAL
# @DESCRIPTION:
# Accepts an environment sysroot, environment prefix (used to locate correct
# binaries for the environment) and environment CPU string (either '' or
# 'BUILD_'), and populates Bazel toolchain targets for the specified
# environment in the given output directory.
bazel_populate_crosstool_target() {
	local env_sysroot="${1}"
	local env_prefix="${2}"
	local cpu_str="${3}"
	local output_dir="${4}"

	# Query compiler type (gcc / clang) from environment variables.
	local comp_type
	comp_type="$("tc-get-${env_prefix}compiler-type" || die)"

	# Get actual compiler binary.
	local comp
	comp="$("tc-get${env_prefix}CC" || die)"

	# Write out the BUILD file for this configuration.
	cpu_str="${cpu_str}" \
	envsubst <<< "${BAZEL_CC_BUILD}" > "${output_dir}/BUILD" || die

	# When using clang, we default to clang-cpp for both CBUILD and CHOST
	# since it's the same binary. When building with gcc, we call tc-getPROG
	# directly for cpp, since we require a program that directly performs
	# preprocessing (i.e. takes no flags), whereas tc-getCPP returns an
	# invocation of the compiler for preprocessing (which uses flags).
	local cpp
	case "${comp_type}" in
		clang) cpp="clang-cpp";;
		gcc) cpp=""tc-get${env_prefix}PROG" CPP cpp";;
		*) die "Unsupported compiler type '${comp_type}'."
	esac

	# Write out the toolchain_config file for this configuration.
	#
	# DWP is defined elsewhere; silence the shellcheck warning.
	# shellcheck disable=SC2154
	cpu_str="${cpu_str}" \
	builtin_include_dirs="$(bazel_get_builtin_include_dirs "${comp}" || die)" \
	env_sysroot="${env_sysroot}" \
	env_cc="$(command -v "${comp}" || die)" \
	env_ar="$(command -v "$("tc-get${env_prefix}AR")" || die)" \
	env_ld="$(command -v "$("tc-get${env_prefix}LD")" || die)" \
	env_cpp="$(command -v "${cpp}" || die)" \
	env_dwp="${DWP}" \
	env_nm="$(command -v "$("tc-get${env_prefix}NM")" || die)" \
	env_objcopy="$(command -v "$("tc-get${env_prefix}OBJCOPY")" || die)" \
	env_objdump="$(command -v "$("tc-get${env_prefix}OBJDUMP")" || die)" \
	env_strip="$(command -v "$("tc-get${env_prefix}STRIP")" || die)" \
	envsubst <<< "${BAZEL_CC_TOOLCHAIN_CONFIG}" > \
	"${output_dir}/cc_toolchain_config.bzl" || die
}

# @FUNCTION: bazel_get_stdlib_linkflag
# @USAGE: <compiler type>
# @RETURN: The correct stdlib linking flag for the given compiler type.
# @MAINTAINER:
# Michael Martis <martis@chromium.org>
# @INTERNAL
bazel_get_stdlib_linkflag() {
	case "${1}" in
	clang) echo "--driver-mode=g++";;
	gcc) echo "-lstdc++";;
	*) die "Unsupported compiler type '${comp_type}'."
	esac
}

# @FUNCTION: bazel_setup_crosstool
# @USAGE: [<host cpu string> <target cpu string>]
# @MAINTAINER:
# Michael Martis <martis@chromium.org>
# @DESCRIPTION:
# Creates Bazel targets (under ${T}) that can be used to configure
# Bazel C++ compilation based on Portage environment variables.
#
# Also updates the bazelrc to specify the new crosstool targets by default.
#
# Should only be called once; subsequent calls will have no effect.
# (Optional) Accepts Bazel "host" and "target" CPU strings as input arguments.
bazel_setup_crosstool() {
	if [[ $# -ne 0 && $# -ne 2 ]]; then
		die "Must give exactly 0 or 2 arguments."
	fi

	if [[ -f "${BAZEL_CC_BAZELRC}" ]]; then
		return
	fi

	bazel_setup_bazelrc

	local host_cpu_str="${1:-$(tc-arch "${CBUILD}")}"
	if [[ -z "${host_cpu_str}" ]]; then
		die "Must specify host CPU string when generating Bazel CROSSTOOL targets."
	fi

	local target_cpu_str="${2:-$(tc-arch "${CHOST}")}"
	if [[ -z "${target_cpu_str}" ]]; then
		die "Must specify target CPU string when generating Bazel CROSSTOOL targets."
	fi

	# Populate host toolchain targets.
	local host_crosstool_dir="${BAZEL_PORTAGE_PACKAGE_DIR}/${BAZEL_CC_CONFIG_DIR}/host"
	mkdir -p "${host_crosstool_dir}" || die
	bazel_populate_crosstool_target / BUILD_ "${host_cpu_str}" "${host_crosstool_dir}"

	# Populate target toolchain targets.
	local target_crosstool_dir="${BAZEL_PORTAGE_PACKAGE_DIR}/${BAZEL_CC_CONFIG_DIR}/target"
	mkdir -p "${target_crosstool_dir}" || die
	bazel_populate_crosstool_target "${PORTAGE_CONFIGROOT}" "" "${target_cpu_str}" "${target_crosstool_dir}"

	# Create a bazelrc specifying the new toolchain targets by default.
	cat > "${BAZEL_CC_BAZELRC}" <<-EOF || die
	# Make Bazel respect Portage C/C++ configuration.
	build --package_path="%workspace%:${BAZEL_PORTAGE_PACKAGE_DIR}"
	build --host_crosstool_top="//${BAZEL_CC_CONFIG_DIR}/host:toolchain" --crosstool_top="//${BAZEL_CC_CONFIG_DIR}/target:toolchain"
	build --host_cpu="${host_cpu_str}" --cpu="${target_cpu_str}" --compiler=local --host_compiler=local
	build --host_platform="//${BAZEL_CC_CONFIG_DIR}/host:${host_cpu_str}_platform"
	build --platforms="//${BAZEL_CC_CONFIG_DIR}/target:${target_cpu_str}_platform"
	build --extra_toolchains="//${BAZEL_CC_CONFIG_DIR}/target:cc-toolchain-${target_cpu_str}"

	# This is super helpful for figuring out how the toolchain is determined
	# build --toolchain_resolution_debug

	# Add correct standard library link flags.
	build --linkopt="$(bazel_get_stdlib_linkflag "$(tc-get-compiler-type)" || die)"
	build --host_linkopt="$(bazel_get_stdlib_linkflag "$(tc-get-BUILD_compiler-type)" || die)"
	EOF

	echo "import ${BAZEL_CC_BAZELRC}" >> "${BAZEL_BAZELRC}" || die

	# Update bazelrc to point to our board build tree.
	cat >> "${BAZEL_BAZELRC}" <<-EOF
	# Some compiler scripts require SYSROOT and PREFIX defined.
	build --action_env SYSROOT="${PORTAGE_CONFIGROOT}"
	build --define=PREFIX="${PORTAGE_CONFIGROOT}/usr"
	EOF

}

# @FUNCTION: cros-bazel-add-rc
# @DESCRIPTION:
# Add lines to the bazelrc configuration preferring the cross-compiler config
# if it is available.
cros-bazel-add-rc() {
	local dest="${BAZEL_CC_BAZELRC}"
	if [[ ! -f "${dest}" ]]; then
		dest="${BAZEL_BAZELRC}"
	fi
	if [[ ! -f "${dest}" ]]; then
		dest=".bazelrc"
	fi
	printf '%s\n' "$@" >> "${dest}" || die
}

# @FUNCTION: cros-bazel-add-copt
# @DESCRIPTION:
# Add --copt config options to the bazelrc preferring the cross-compiler config
# if it is available.
cros-bazel-add-copt() {
	cros-bazel-add-rc "${@/#/build --copt=}"
}

# @FUNCTION: cros-bazel-add-cxxopt
# @DESCRIPTION:
# Add --cxxopt config options to the bazelrc preferring the cross-compiler
# config if it is available.
cros-bazel-add-cxxopt() {
	cros-bazel-add-rc "${@/#/build --cxxopt=}"
}

# @FUNCTION: cros-bazel-add-linkopt
# @DESCRIPTION:
# Add --linkopt config options to the bazelrc preferring the cross-compiler
# config if it is available.
cros-bazel-add-linkopt() {
	cros-bazel-add-rc "${@/#/build --linkopt=}"
}

# If an overlay has eclass overrides, but doesn't actually override this
# eclass, we'll have ECLASSDIR pointing to the active overlay's
# eclass/ dir, but this eclass is still in the main chromiumos tree.  So
# add a check to locate the cros-bazel/ regardless of what's going on.
CROS_BAZEL_ECLASSDIR_LOCAL=${BASH_SOURCE[0]%/*}
cros-bazel_eclass_dir() {
	# shellcheck disable=SC2154
	local d="${ECLASSDIR}/cros-bazel"
	if [[ ! -d ${d} ]] ; then
		d="${CROS_BAZEL_ECLASSDIR_LOCAL}/cros-bazel"
	fi
	echo "${d}"
}

# @FUNCTION: bazel_setup_system_protobuf
# @DESCRIPTION:
# Sets up the cc_toolchain rule for protobuf using PKG_CONFIG so the
# cflags and libs are correct. This should be called after the .bazelrc
# file is created.
bazel_setup_system_protobuf() {
	tc-export PKG_CONFIG

	local copts
	local link_opts
	copts="$("$(cros-bazel_eclass_dir)/pkg-config-to-bazel.py" "${PKG_CONFIG}" protobuf --cflags)"
	link_opts="$("$(cros-bazel_eclass_dir)/pkg-config-to-bazel.py" "${PKG_CONFIG}" protobuf --libs)"
	[[ -z "${link_opts}" ]] && die "system protobuf bazel build template failed (${PKG_CONFIG})"
	mkdir -p "${S}/cros-bazel/protobuf/" || die
	sed -e "s;\%COPTS\%;${copts};g" \
		-e "s;\%LINK_OPTS\%;${link_opts};g" \
		"$(cros-bazel_eclass_dir)/protobuf/BUILD.bazel.in" \
		> "${S}/cros-bazel/protobuf/BUILD.bazel" || die

	cros-bazel-add-rc ''
	cros-bazel-add-rc '# Use system protobuf.'
	cros-bazel-add-rc 'build --proto_toolchain_for_cc=//cros-bazel/protobuf:system_cc_toolchain'
}

_CROS_BAZEL_ECLASS=1
fi
