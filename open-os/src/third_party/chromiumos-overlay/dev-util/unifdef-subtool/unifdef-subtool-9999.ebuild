# Copyright 2024 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

EAPI=7

CROS_WORKON_PROJECT="chromiumos/infra/build/empty-project"
CROS_WORKON_LOCALNAME="../platform/empty-project"
CROS_WORKON_OUTOFTREE_BUILD=1

inherit cros-subtool cros-workon

DESCRIPTION="Subtools definition to export unifdef from dev-util/unifdef."
HOMEPAGE="https://www.chromium.org/chromium-os/developer-library/guides/portage/subtools-builder/"

LICENSE="BSD-Google"
SLOT="0"
KEYWORDS="~*"
IUSE=""

RDEPEND="dev-util/unifdef"

src_install() {
	cros-subtool_src_install "${FILESDIR}/unifdef.textproto"
}
