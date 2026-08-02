# Copyright 2024 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

EAPI=7

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
KEYWORDS="~*"
# This package has no unittests.
RESTRICT="test"

RDEPEND="
	dev-cpp/gtest:=
	media-libs/skia:="

DEPEND="${RDEPEND}"

BDEPEND="virtual/pkgconfig"
