# Copyright 2024 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2.

# @ECLASS: cros-gcc.eclass
# @MAINTAINER:
# ChromeOS toolchain team.<chromeos-toolchain@google.com>

# @DESCRIPTION:
# Inherit this eclass if your package needs to build with gcc or
# requires a GCC-derived library (e.g., libgcc).

if [[ -z "${_ECLASS_CROS_GCC}" ]]; then
_ECLASS_CROS_GCC=1

case ${EAPI:-0} in
[0123456]) die "Unsupported EAPI=${EAPI:-0} (too old) for ${ECLASS}" ;;
esac

IUSE="cros_host"

# TODO(b/316038270): Add binutils.
BDEPEND="
	cros_host? ( sys-devel/gcc )
	!cros_host? (
		arm? ( cross-armv7a-cros-linux-gnueabihf/gcc )
		arm64? ( cross-aarch64-cros-linux-gnu/gcc )
		amd64? ( cross-x86_64-cros-linux-gnu/gcc )
	)
"

# @ECLASS-VARIABLE: CROS_ENABLE_CBUILD_GCC
# @PRE_INHERIT
# @DESCRIPTION:
# If set to 1, adds the CBUILD gcc (i.e., sys-devel/gcc) as a BDEPEND. This is
# only necessary if your package needs to compile build tools with gcc while
# cross-compiling a package.
: "${CROS_ENABLE_CBUILD_GCC:=0}"

if [[ "${CROS_ENABLE_CBUILD_GCC}" -eq 1 ]]; then
	BDEPEND+="
		!cros_host? ( sys-devel/gcc )
	"
fi

fi # _ECLASS_CROS_GCC
