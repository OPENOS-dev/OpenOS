# Copyright 2024 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

DESCRIPTION="List of packages that are needed by TFLite Stable Delegate"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform/tflite/"

LICENSE="metapackage"
SLOT="0"
KEYWORDS="*"

# Empty by default. Each overlay can override this with its own dependencies.
RDEPEND=""
