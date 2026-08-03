# Copyright 2014 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7
CROS_WORKON_COMMIT="5a90059453215580699958dc114f3201cbd24e14"
CROS_WORKON_TREE="9196c1ff8baefa7e3d7d296a51b157fa16439119"
CROS_WORKON_PROJECT="chromiumos/third_party/autotest"
CROS_WORKON_LOCALNAME="third_party/autotest/files"

inherit cros-workon autotest

DESCRIPTION="Cryptohome autotests"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/third_party/autotest/"
SRC_URI=""

LICENSE="GPL-2"
SLOT="0"
KEYWORDS="*"
# Enable autotest by default.
IUSE="+autotest"

COMMON_DEPEND="
	!<chromeos-base/autotest-tests-0.0.3
"

RDEPEND="
	${COMMON_DEPEND}
	chromeos-base/cryptohome-dev-utils
"

DEPEND="
	${COMMON_DEPEND}
"

IUSE_TESTS="
	+tests_platform_CryptohomeFio
"

IUSE="${IUSE} ${IUSE_TESTS}"

AUTOTEST_FILE_MASK="*.a *.tar.bz2 *.tbz2 *.tgz *.tar.gz"
