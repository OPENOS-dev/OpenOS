# Copyright 2024 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_WORKON_COMMIT="e67643c64a105f6f744b007eb857f381ace07e8e"
CROS_WORKON_TREE=("518b50f8b6d01e95cbd933487ed7c6452ac4acb3" "ed617d85acd2502aef6ce76950ff0449c5596aec" "f91b6afd5f2ae04ee9a2c19109a3a4a36f7659e6")
CROS_WORKON_INCREMENTAL_BUILD="1"
CROS_WORKON_LOCALNAME="platform2"
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_OUTOFTREE_BUILD=1
CROS_WORKON_SUBTREE="common-mk dbus_perfetto_producer .gn"

PLATFORM_SUBDIR="dbus_perfetto_producer"
WANT_LIBCHROME="yes"
WANT_LIBBRILLO="no"

inherit cros-workon platform

DESCRIPTION="D-bus Event Producer of Perfetto for Chromium OS."
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform2/+/HEAD/dbus_perfetto_producer"

LICENSE="BSD-Google"
KEYWORDS="*"

DEPEND="
	chromeos-base/perfetto:=
	sys-apps/dbus:=
"

RDEPEND="${DEPEND}"
