# Copyright 2024 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI="7"

# This ebuild only cares about its own FILESDIR and ebuild file, so it tracks
# the canonical empty project.
CROS_WORKON_COMMIT="d2d95e8af89939f893b1443135497c1f5572aebc"
CROS_WORKON_TREE="776139a53bc86333de8672a51ed7879e75909ac9"
CROS_WORKON_PROJECT="chromiumos/infra/build/empty-project"
CROS_WORKON_LOCALNAME="../platform/empty-project"
CROS_WORKON_OUTOFTREE_BUILD=1

inherit cros-workon

DESCRIPTION="
	List of build-time packages that are needed for building boards.
	These are built on a per-board basis instead of part of the SDK.
	Generally first party projects that are updated frequently should be here.
	'broot' is short for 'build root'; see the portage BROOT variable.
"
HOMEPAGE="https://dev.chromium.org/"

LICENSE="metapackage"
SLOT="0"
KEYWORDS="*"
IUSE=""

RDEPEND="
	chromeos-base/chromeos-dbus-bindings
	chromeos-base/tast-cmd
	chromeos-base/tast-remote-tests
	dev-libs/flatbuffers
	dev-util/test-services
	dev-util/wayland-scanner
"

# Disable default install rules (e.g. docs).
src_install() { :; }
