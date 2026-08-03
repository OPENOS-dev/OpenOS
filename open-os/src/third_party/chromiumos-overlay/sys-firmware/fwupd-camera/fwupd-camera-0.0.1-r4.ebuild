# Copyright 2023 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI="7"

# This ebuild only cares about its own FILESDIR and ebuild file, so it tracks
# the canonical empty project.
CROS_WORKON_COMMIT="d2d95e8af89939f893b1443135497c1f5572aebc"
CROS_WORKON_TREE="776139a53bc86333de8672a51ed7879e75909ac9"
CROS_WORKON_PROJECT="chromiumos/infra/build/empty-project"
CROS_WORKON_LOCALNAME="../platform/empty-project"
CROS_WORKON_OUTOFTREE_BUILD=1

inherit cros-workon cros-fwupd

DESCRIPTION="Installs camera firmware files used by fwupd."
KEYWORDS="*"

FILENAMES=(
	"303bef0da8ff6acd8590bfcebdbdbc8e13bfebd63ca81df5bb6796b8b68c45d5-20230724_YHVA-3_RTS5856_OV2740_Chrome_acerR1-15_cache.cab"
	"9cfa3af86e5e72df127fecd43c6255ee7bf3b3516a26c94bad0e3a21038baefd-firmware_v0004_1.cab"
	"b98f12abc69cae57e65f2f436cb31a4562766fbbc1539aef563feec42756809b-FW0_v1112_CAP.cab"
)
SRC_URI="${FILENAMES[*]/#/${CROS_FWUPD_URL}/}"
LICENSE="LVFS-Vendor-Agreement-v1"

DEPEND=""
RDEPEND="
	!sys-firmware/realtek-camera-firmware
	sys-apps/fwupd
"
