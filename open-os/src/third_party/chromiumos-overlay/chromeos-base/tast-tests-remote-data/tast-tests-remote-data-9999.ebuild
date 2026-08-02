# Copyright 2021 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

EAPI=7

CROS_WORKON_PROJECT=(
	"chromiumos/platform/tast-tests"
	"chromiumos/platform/fw-testing-configs"
)
CROS_WORKON_LOCALNAME=(
	"platform/tast-tests"
	"platform/fw-testing-configs"
)
CROS_WORKON_DESTDIR=(
	"${S}/tast-tests"
	"${S}/fw-testing-configs"
)

# There are symlinks between remote data to local data, so we can't make the
# subtree "src/go.chromium.org/tast-tests/cros"/remote".
# TODO(oka): have a clear separation between local and remote, and make that
# happen.
CROS_WORKON_SUBTREE=("src/go.chromium.org/tast-tests/cros" "")
# shellcheck disable=SC2034
TAST_BUNDLE_ROOT="go.chromium.org/tast-tests/cros"

inherit cros-workon tast-bundle-data

DESCRIPTION="Data files for remote Tast tests"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform/tast-tests"

LICENSE="BSD-Google GPL-3"
SLOT="0/0"
KEYWORDS="~*"

RDEPEND="!<chromeos-base/tast-remote-tests-cros-0.0.2"

src_install() {
	# Copy CONSOLIDATED.json to src/go.chromium.org/tast-tests/cros/remote/firmware/data
	# because the symlink doesn't exist when using cros_workon
	if [[ ! -d "${S}/tast-tests/src/go.chromium.org/tast-tests/cros/remote/firmware/data" ]]; then
		mkdir "${S}/tast-tests/src/go.chromium.org/tast-tests/cros/remote/firmware/data"
	fi
	cp --remove-destination "${S}/fw-testing-configs/CONSOLIDATED.json" \
		"${S}/tast-tests/src/go.chromium.org/tast-tests/cros/remote/firmware/data/CONSOLIDATED.json" || die
	pushd tast-tests || die
	tast-bundle-data_src_install
	popd || die
}
