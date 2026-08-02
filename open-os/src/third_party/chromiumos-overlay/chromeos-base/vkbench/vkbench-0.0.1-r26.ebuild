# Copyright 2020 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

EAPI=7

CROS_WORKON_COMMIT="8673541bd5c1cd6d1002878bb53d366a9c835ac6"
CROS_WORKON_TREE="ebb414eb8bd4f286b826b51ddede99c225848612"
CROS_WORKON_LOCALNAME="platform/vkbench"
CROS_WORKON_PROJECT="chromiumos/platform/vkbench"

inherit cros-workon cmake

DESCRIPTION="Microbenchmark for vulkan"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform/vkbench/"
SRC_URI=""

LICENSE="BSD-Google"
KEYWORDS="*"
IUSE="vulkan"

REQUIRED_USE="vulkan"

BDEPEND="
	dev-util/glslang
"

RDEPEND="
	dev-libs/libfmt:=
	media-libs/libpng:=
	media-libs/vulkan-loader:=
	virtual/vulkan-icd:=
"
DEPEND="${RDEPEND}
	dev-util/vulkan-headers"
