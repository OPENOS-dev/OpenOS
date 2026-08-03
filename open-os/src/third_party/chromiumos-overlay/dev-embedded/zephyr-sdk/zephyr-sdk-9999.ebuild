# Copyright 2024 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

EAPI=7

CROS_WORKON_PROJECT="chromiumos/infra/build/empty-project"
CROS_WORKON_LOCALNAME="../platform/empty-project"
CROS_WORKON_OUTOFTREE_BUILD=1

inherit cros-workon

LICENSE="BSD-Google"
KEYWORDS="~*"
SLOT="0"
DESCRIPTION="Zephyr SDK toolchain installer"
IUSE=""

SDK_INSTANCE_ID="7b5xSTp1KccAYDvo9pumqPB0ZDRlep5bs21soXBpV-kC"
SDK_VERSION="0.16.8"
SRC_URI="cipd://infra/3pp/tools/zephyr_sdk/linux-amd64:${SDK_INSTANCE_ID} -> ${P}-amd64.zip"

BDEPEND="app-arch/unzip"
RDEPEND="
	dev-lang/python:3.8
	sys-devel/arc-toolchain-r
"

RESTRICT="mirror"

S="${WORKDIR}"

TARGETS_TO_INSTALL=(
	"riscv64"
	"x86_64"
)

src_unpack() {
	unpack "${P}-amd64.zip"
	ls -l "${S}"
}

src_install() {
	local target

	insinto "/opt/zephyr-sdk-${SDK_VERSION}/" || die

	for target in "${TARGETS_TO_INSTALL[@]}"; do
		# Install each target directory
		doins -r "${WORKDIR}/${target}-zephyr-elf" || die
	done

	# Copy cmake directory
	doins -r "${WORKDIR}/cmake" || die

	# Copy the sdk_version file
	doins "${WORKDIR}/sdk_version" || die
}
