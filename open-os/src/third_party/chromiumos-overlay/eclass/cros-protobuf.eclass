# Copyright 2023 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

# @ECLASS: protobuf.eclass
# @MAINTAINER:
# Protobuf OWNERS
# @VCSURL: https://chromium.googlesource.com/chromiumos/overlays/chromiumos-overlay/+/HEAD/eclass/@ECLASS@
# @BLURB: eclass to handle protobuf and its public dependencies with their subslots.
# @DESCRIPTION:
# Protobuf 22.0+ requires reverse dependencies to link abseil-cpp so both
# need to be included together.

if [[ -z "${_ECLASS_CROS_PROTOBUF}" ]]; then
_ECLASS_CROS_PROTOBUF=1

# @ECLASS-VARIABLE: CROS_PROTOBUF_DEPS
# @DESCRIPTION:
# CONSTANT!
# DEPENDs and RDEPENDs for protobuf.
CROS_PROTOBUF_DEPS="
	dev-cpp/abseil-cpp:=
	dev-libs/protobuf:=
"

# @ECLASS-VARIABLE: CROS_PROTOC_DEPS
# @DESCRIPTION:
# CONSTANT!
# BDEPENDs for protoc.
CROS_PROTOC_DEPS="
	dev-libs/protobuf
"

# @ECLASS-VARIABLE: CROS_PROTOBUF_APPLY_DEFAULT_DEPS
# @DESCRIPTION:
# If true protobuf is added to DEPEND, RDEPEND, and BDEPEND automatically.
#
# This needs to be set before inheriting the eclass.
: "${CROS_PROTOBUF_APPLY_DEFAULT_DEPS:=1}"

if [[ "${CROS_PROTOBUF_APPLY_DEFAULT_DEPS}" == "1" ]]; then
	# For users of protoc.
	BDEPEND="${CROS_PROTOC_DEPS}"

	DEPEND="${CROS_PROTOBUF_DEPS}"
	RDEPEND="${CROS_PROTOBUF_DEPS}"
fi

fi  # _ECLASS_CROS_PROTOBUF
