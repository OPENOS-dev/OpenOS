# Copyright 2023 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

EAPI=7

CROS_WORKON_COMMIT="e67643c64a105f6f744b007eb857f381ace07e8e"
CROS_WORKON_TREE=("39a0b4536f8ba7c11af8a3264a980d6dda237b3a" "518b50f8b6d01e95cbd933487ed7c6452ac4acb3" "f91b6afd5f2ae04ee9a2c19109a3a4a36f7659e6")
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
KEYWORDS="*"
IUSE=""

RDEPEND="sys-boot/kdump-init
	sys-kernel/chromeos-kernel-kdump-5_15"

src_install() {
	platform_src_install
}
