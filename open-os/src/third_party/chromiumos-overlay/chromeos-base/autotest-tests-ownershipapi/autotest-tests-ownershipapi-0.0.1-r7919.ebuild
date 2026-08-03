# Copyright 2011 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_WORKON_COMMIT="59b04907bf0053c7e68012a7a8b7b1949f1b1554"
CROS_WORKON_TREE="2171775a95163b538a6fc241317327290e42e555"
PYTHON_COMPAT=( python3_11 )

CROS_WORKON_PROJECT="chromiumos/third_party/autotest"

inherit cros-workon autotest python-any-r1

DESCRIPTION="login_OwnershipApi autotest"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/third_party/autotest/"
SRC_URI=""

LICENSE="GPL-2"
SLOT="0"
KEYWORDS="*"
IUSE="+xset +tpmtools"
# Enable autotest by default.
IUSE="${IUSE} +autotest"

COMMON_DEPEND="
	chromeos-base/telemetry
"

RDEPEND="${RDEPEND}
	${COMMON_DEPEND}
	chromeos-base/autotest-deps-policy
	dev-python/protobuf-python
	dev-python/pygobject
"

DEPEND="${COMMON_DEPEND}"

BDEPEND="
	dev-python/dbus-python
	dev-python/packaging
	dev-python/pygobject
	dev-python/requests
"

# The telemetry dependency comes from the chrome.py import in some of the tests.
IUSE_TESTS="
	+tests_login_CryptohomeOwnerQuery
"

IUSE="${IUSE} ${IUSE_TESTS}"

CROS_WORKON_LOCALNAME="third_party/autotest/files"

AUTOTEST_DEPS_LIST=""
AUTOTEST_CONFIG_LIST=""
AUTOTEST_PROFILERS_LIST=""

AUTOTEST_FILE_MASK="*.a *.tar.bz2 *.tbz2 *.tgz *.tar.gz"

src_prepare() {
	# Telemetry tests require the path to telemetry source to exist in order to
	# build.  Copy the telemetry source to a temporary directory that is writable,
	# so that file removals in Telemetry source can be performed properly.
	export TMP_DIR="$(mktemp -d)"
	cp -r "${SYSROOT}/usr/local/telemetry" "${TMP_DIR}"
	export PYTHONPATH="${TMP_DIR}/telemetry/src/third_party/catapult/telemetry"
	autotest_src_prepare
}
