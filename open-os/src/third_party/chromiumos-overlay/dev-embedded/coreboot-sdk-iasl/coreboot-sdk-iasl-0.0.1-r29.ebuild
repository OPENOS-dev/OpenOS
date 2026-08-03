# Copyright 2024 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

EAPI=7

CROS_WORKON_COMMIT="82390cc046aaa9e947f1e5261269204c6ea4122d"
CROS_WORKON_TREE="bc79451ecc77e95c4a0ef3b049d10b4355d09b84"
CROS_WORKON_PROJECT="chromiumos/third_party/coreboot"
CROS_WORKON_LOCALNAME="coreboot"
CROS_WORKON_SUBTREE="util/crossgcc"

inherit cros-workon coreboot-sdk-build coreboot-sdk-versions

DESCRIPTION="ACPI compiler from coreboot-sdk"
HOMEPAGE="https://www.coreboot.org"
LICENSE="GPL-3 LGPL-3"
KEYWORDS="*"
BDEPEND="dev-embedded/coreboot-sdk-bootstrap"

# shellcheck disable=SC2154
SRC_URI="${IASL_SRC_URI}"

src_compile() {
	coreboot-sdk-build_buildgcc --package IASL
}
