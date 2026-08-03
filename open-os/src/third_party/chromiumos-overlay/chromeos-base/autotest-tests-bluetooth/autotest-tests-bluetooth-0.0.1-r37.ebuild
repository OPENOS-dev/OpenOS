# Copyright 2022 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI="7"

CROS_WORKON_COMMIT="4f418e10028c6bdc987dcde2fa67f345b024754d"
CROS_WORKON_TREE="8c18267aa0d6fee0896969c3d256a25fd7b6fb56"
PYTHON_COMPAT=( python3_11 )

CROS_WORKON_PROJECT="chromiumos/third_party/autotest"
CROS_WORKON_LOCALNAME="third_party/autotest/files"

inherit cros-workon autotest python-any-r1

DESCRIPTION="Autotest tests for Bluetooth"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/third_party/autotest/"
SRC_URI=""
LICENSE="GPL-2"
SLOT="0"
KEYWORDS="*"

# Enable autotest by default.
IUSE="+autotest"

RDEPEND="
	chromeos-base/autotest-client
	dev-python/btsocket
"

BDEPEND="
	dev-python/btsocket
	dev-python/dbus-python
	dev-python/numpy
	dev-python/protobuf-python
	dev-python/pydbus
	dev-python/pygobject
"

CLIENT_IUSE_TESTS="
	+tests_bluetooth_AVLHCI
	+tests_bluetooth_AVLDriver
	+tests_bluetooth_AdapterQuickHealthClient
"

IUSE_TESTS="${IUSE_TESTS}
	${CLIENT_IUSE_TESTS}
"

IUSE="${IUSE} ${IUSE_TESTS}"

AUTOTEST_FILE_MASK="*.a *.tar.bz2 *.tbz2 *.tgz *.tar.gz"
