# Copyright 2017 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

PYTHON_COMPAT=( python3_{6..11} )

CROS_WORKON_LOCALNAME="platform2"
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_OUTOFTREE_BUILD="1"
CROS_WORKON_SUBTREE="common-mk metrics update_engine .gn"

inherit cros-workon python-r1

DESCRIPTION="ChromeOS Update Engine Update Payload Scripts"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform2/+/HEAD/update_engine/"

LICENSE="Apache-2.0"
KEYWORDS="~*"

RDEPEND="${PYTHON_DEPS}
	dev-python/protobuf-python[${PYTHON_USEDEP}]
"
DEPEND="${PYTHON_DEPS}"

src_install() {
	# Install update_payload scripts.
	install_update_payload() {
		# TODO(crbug.com/771085): Clear the SYSROOT var as python will use
		# that to define the sitedir which means we end up installing into
		# a path like /build/$BOARD/build/$BOARD/xxx.  This is a bug in the
		# core python logic, but this is breaking moblab, so hack it for now.
		insinto "$(python_get_sitedir | sed "s:^${SYSROOT}::")/update_payload"
		# shellcheck disable=SC2046
		doins $(printf '%s\n' update_engine/scripts/update_payload/*.py | grep -v unittest)
		doins update_engine/scripts/update_payload/update-payload-key.pub.pem
	}
	python_foreach_impl install_update_payload

	# Install paycheck.py script as check_update_payload.
	newbin update_engine/scripts/paycheck.py check_update_payload
}

src_test() {
	# Run update_payload unittests.
	cd "${S}"/update_engine/scripts || die
	python_test() {
		./run_unittests || die
	}
	python_foreach_impl python_test
}
