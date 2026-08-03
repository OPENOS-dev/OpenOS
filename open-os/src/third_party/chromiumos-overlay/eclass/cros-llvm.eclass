# Copyright 2017 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2.

# @ECLASS: cros-llvm.eclass
# @MAINTAINER:
# ChromeOS toolchain team.<chromeos-toolchain@google.com>

# @DESCRIPTION:
# Functions to set the right toolchains and install prefix for llvm
# related libraries in crossdev stages.

inherit multilib

if [[ ${CATEGORY} == cross-* ]] ; then
	DEPEND="
		${CATEGORY}/binutils
		sys-devel/llvm
		"
fi

export CBUILD=${CBUILD:-${CHOST}}
export CTARGET=${CTARGET:-${CHOST}}

if [[ "${CTARGET}" = "${CHOST}" ]] ; then
	if [[ "${CATEGORY/cross-}" != "${CATEGORY}" ]] ; then
		export CTARGET=${CATEGORY/cross-}
	fi
fi

setup_cross_toolchain() {
	export CC="${CBUILD}-clang"
	export CXX="${CBUILD}-clang++"
	export PREFIX="/usr"

	if [[ ${CATEGORY} == cross-* ]] ; then
		export CC="${CTARGET}-clang"
		export CXX="${CTARGET}-clang++"
		export PREFIX="/usr/${CTARGET}/usr"
		export AS="$(tc-getAS "${CTARGET}")"
		export STRIP="$(tc-getSTRIP "${CTARGET}")"
		export OBJCOPY="$(tc-getOBJCOPY "${CTARGET}")"
	elif [[ "${CTARGET}" != "${CBUILD}" ]] ; then
		export CC="${CTARGET}-clang"
		export CXX="${CTARGET}-clang++"
	fi
	unset ABI MULTILIB_ABIS DEFAULT_ABI
	multilib_env "${CTARGET}"
}

# @FUNCTION cros-llvm_get_most_recent_revision
# @DESCRIPTION:
# Returns the revision number (e.g., 12345) for the currently checked out LLVM
# revision.
#
# shellcheck disable=SC2120
cros-llvm_get_most_recent_revision() {
	cat "${S}/cros/llvm-rev" || die
}

is_baremetal_abi() {
	# ABIs like armv7m-cros-eabi, arm-none-eabi, riscv32-cros-elf, etc.
	if [[ "${CTARGET}" == *-eabi || "${CTARGET}" == riscv32-* ]]; then
		return 0
	fi
	return 1
}

# @FUNCTION cros-llvm_ensure_patches_applied
# @DESCRIPTION:
# Ensures that patches are applied, by either applying them (in non-9999
# ebuilds), or checking for a stamp that verifies that the patches were applied
# (in 9999 ebuilds).
cros-llvm_ensure_patches_applied() {
	local f="${S}/cros/README.md"
	[[ -e "${f}" ]] || \
		die "No base commit stamp found at ${f}. Did you run src/third_party/toolchain-utils/py/llvm_tools/ready_llvm_branch.py?"
}
