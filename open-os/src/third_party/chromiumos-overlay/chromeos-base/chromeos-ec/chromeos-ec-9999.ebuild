# Copyright 2012 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE.makefile file.

# A note about this ebuild: this ebuild is Unified Build enabled but
# not in the way in which most other ebuilds with Unified Build
# knowledge are: the primary use for this ebuild is for engineer-local
# work or firmware builder work. In both cases, the build might be
# happening on a branch in which only one of many of the models are
# available to build. The logic in this ebuild succeeds so long as one
# of the many models successfully builds.

EAPI=7

CROS_WORKON_PROJECT=(
	"chromiumos/platform/ec"
	"chromiumos/third_party/cryptoc"
	"external/gitlab.com/libeigen/eigen"
	"external/gob/boringssl/boringssl"
	"external/github.com/google/googletest"
)
CROS_WORKON_LOCALNAME=(
	"platform/ec-legacy"
	"third_party/cryptoc"
	"third_party/eigen3"
	"third_party/boringssl"
	"third_party/googletest"
)

CROS_WORKON_EGIT_BRANCH=(
	"ec-legacy"
	"main"
	"upstream/master"
	"upstream/master"
	"main"
)

CROS_WORKON_DESTDIR=(
	"${S}/platform/ec-legacy"
	"${S}/third_party/cryptoc"
	"${S}/third_party/eigen3"
	"${S}/third_party/boringssl"
	"${S}/third_party/googletest"
)

inherit cros-ec cros-workon coreboot-sdk coreboot-sdk-ec-dependencies

# TODO(b/332378804): Compile build host tools with clang.
BDEPEND="
	sys-devel/gcc
"

# Make sure config tools use the latest schema.
BDEPEND+="
	>=chromeos-base/chromeos-config-host-0.0.2
"

# coreboot-sdk cross-compilers are used by the EC build.
# TODO - b/330748516: Some boards may not need all of these cross-compilers.  We
# could use USE flags to determine the required toolchains for each board.
BDEPEND+=""

DESCRIPTION="Embedded Controller firmware code"
KEYWORDS="~*"

# strip: chromeos-ec package installs unstrippable firmware.
# mirror: We fetch toolchains from gs://chromiumos-sdk.
RESTRICT="strip mirror"

coreboot-sdk_enable nds32le-elf
coreboot-sdk_enable i386-elf
coreboot-sdk_enable arm-eabi
coreboot-sdk_enable riscv-elf

src_compile() {
	export COREBOOT_SDK_ROOT_arm="${COREBOOT_SDK_PREFIX}"
	export COREBOOT_SDK_ROOT_nds32="${COREBOOT_SDK_PREFIX}"
	export COREBOOT_SDK_ROOT_riscv="${COREBOOT_SDK_PREFIX}"
	export COREBOOT_SDK_ROOT_x86="${COREBOOT_SDK_PREFIX}"
	export CROSS_COMPILE_CC_NAME=gcc

	cros-ec_src_compile
}
