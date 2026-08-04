# Copyright 2024 The ChromiumOS Authors.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

EAPI=7

DESCRIPTION="Virtual for OpenGLES implementations"

LICENSE="metapackage"
SLOT="0"
KEYWORDS="-* arm arm64"
IUSE=""

DEPEND="
	media-libs/mali-drivers-valhall-bin
	x11-drivers/opengles-headers
"
RDEPEND="${DEPEND}"
BDEPEND=""
