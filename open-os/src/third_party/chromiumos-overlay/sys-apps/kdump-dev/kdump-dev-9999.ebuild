# Copyright 2023 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

EAPI=7

CROS_WORKON_LOCALNAME="../platform2"
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_DESTDIR="${S}/platform2"
CROS_WORKON_SUBTREE="kdump common-mk .gn"

PLATFORM_SUBDIR="kdump"
WANT_LIBCHROME="no"

inherit cros-workon platform

DESCRIPTION="Install ChromeOS Kdump for developers to test image"

LICENSE="BSD-Google"
SLOT="0"
KEYWORDS="~*"
IUSE=""

RDEPEND="sys-boot/kdump-init
	sys-kernel/chromeos-kernel-kdump-5_15"

src_install() {
	platform_src_install
}
