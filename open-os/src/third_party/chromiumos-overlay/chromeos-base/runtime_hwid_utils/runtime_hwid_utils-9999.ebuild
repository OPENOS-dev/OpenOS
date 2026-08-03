# Copyright 2025 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

EAPI=7

CROS_WORKON_INCREMENTAL_BUILD=1
CROS_WORKON_LOCALNAME="platform2"
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_OUTOFTREE_BUILD=1
CROS_WORKON_SUBTREE="common-mk hardware_verifier libcrossystem .gn"

PLATFORM_SUBDIR="hardware_verifier/runtime_hwid_utils"

inherit cros-workon platform

DESCRIPTION="Runtime HWID Utility Tool/Lib for Chrome OS"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform2/+/HEAD/hardware_verifier/runtime_hwid_utils/"

LICENSE="BSD-Google"
KEYWORDS="~*"

COMMON_DEPEND="
	chromeos-base/libcrossystem:=
"

RDEPEND="${COMMON_DEPEND}"

DEPEND="${COMMON_DEPEND}"
