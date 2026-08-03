# Copyright 2024 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

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
KEYWORDS="~*"

DEPEND="
	chromeos-base/perfetto:=
	sys-apps/dbus:=
"

RDEPEND="${DEPEND}"
