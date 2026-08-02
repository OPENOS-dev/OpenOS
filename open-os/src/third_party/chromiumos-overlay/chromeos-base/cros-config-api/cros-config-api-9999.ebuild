# Copyright 2020 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

EAPI=7

CROS_WORKON_PROJECT="chromiumos/config"
CROS_WORKON_LOCALNAME="config"
CROS_WORKON_SUBTREE="python go test"

PYTHON_COMPAT=( python3_{6..11} )
DISTUTILS_USE_SETUPTOOLS=bdepend

CROS_GO_PACKAGES=(
	"go.chromium.org/chromiumos/config/go/..."
)

inherit cros-go cros-workon distutils-r1

DESCRIPTION="Provides python and go bindings to the config API"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/config/+/HEAD/python/"

LICENSE="BSD-Google"
SLOT=0
KEYWORDS="~*"
# Disable unittesting for client bindings.
RESTRICT="test"

DEPEND="
	dev-go/genproto
	dev-go/grpc
	dev-go/protobuf-legacy-api
"

RDEPEND="
	${DEPEND}
	dev-python/lxml[${PYTHON_USEDEP}]
	dev-python/protobuf-python[${PYTHON_USEDEP}]
"

src_unpack() {
	cros-workon_src_unpack
	# distutils-r1 provides src_configure, src_install and src_test steps for
	# python bindings, and they require S to be set to the Python source base
	# directory.
	S+="/python"

	# cros-go requires the Go workspace be set if it is not directly in ${S},
	# like in this case where ${S} is set to python above.
	CROS_GO_WORKSPACE="${S}/../go"
}

src_install() {
	distutils-r1_src_install
	cros-go_src_install
}
