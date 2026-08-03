# Copyright 2024 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

EAPI=7

CROS_WORKON_COMMIT="e67643c64a105f6f744b007eb857f381ace07e8e"
CROS_WORKON_TREE=("f91b6afd5f2ae04ee9a2c19109a3a4a36f7659e6" "717e2f0156fff057534e14f3107db367feeaa53d" "518b50f8b6d01e95cbd933487ed7c6452ac4acb3")
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_LOCALNAME="../platform2"
CROS_WORKON_SUBTREE=".gn camera/features/ocr common-mk"
CROS_WORKON_OUTOFTREE_BUILD="1"
CROS_WORKON_INCREMENTAL_BUILD="1"

PLATFORM_SUBDIR="camera/features/ocr"

inherit cros-workon platform

DESCRIPTION="Chrome Screen AI OCR test"

LICENSE="BSD-Google"
SLOT="0"
KEYWORDS="*"
# This package has no unittests.
RESTRICT="test"

RDEPEND="
	dev-cpp/gtest:=
	media-libs/skia:="

DEPEND="${RDEPEND}"

BDEPEND="virtual/pkgconfig"
