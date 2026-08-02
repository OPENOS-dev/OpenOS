# Copyright 2023 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_WORKON_COMMIT="4038229a5b6e7265cddaab63466b626102fa6577"
CROS_WORKON_TREE="df8e2ffeb6e63ccd6817065bce8f9f2fe0f19f0c"
CROS_WORKON_LOCALNAME="platform2"
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_OUTOFTREE_BUILD=1
CROS_WORKON_INCREMENTAL_BUILD=1
CROS_WORKON_SUBTREE="modemloggerd"

PLATFORM_SUBDIR="modemloggerd"

inherit cros-workon platform tmpfiles

DESCRIPTION="Chrome OS modem logger for release images"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform2/+/HEAD/modemloggerd"

LICENSE="BSD-Google"
KEYWORDS="*"
IUSE=""
RESTRICT="test"

src_configure() {
	:
}

src_compile() {
	:
}

src_install() {
	# modemloggerd does not meet security and privacy requirements for release images,
	# so the binary must not be installed on release images. However, the upstart conf
	# needs to reside in /etc/init/, and not in /usr/local/etc/init/, thus we need this
	# ebuild. In the future, this ebuild may be used for building a version of
	# modemloggerd for modems that clear security and privacy checks.
	insinto /etc/init
	doins init/modemloggerd.conf

	insinto /usr/share/minijail
	doins minijail/modemloggerd.conf

	# modemloggerd-dev ebuild will install the binary

	dotmpfiles tmpfiles.d/*.conf
}
