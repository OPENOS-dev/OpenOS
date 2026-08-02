# Copyright 2024 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI="7"

CROS_WORKON_COMMIT="d2d95e8af89939f893b1443135497c1f5572aebc"
CROS_WORKON_TREE="776139a53bc86333de8672a51ed7879e75909ac9"
CROS_WORKON_PROJECT="chromiumos/infra/build/empty-project"
CROS_WORKON_LOCALNAME="../platform/empty-project"
CROS_WORKON_OUTOFTREE_BUILD=1

inherit cros-workon

DESCRIPTION="List of packages updated by the SDK subtools builder before it
looks for subtool definitions to bundle and upload."
HOMEPAGE="https://www.chromium.org/chromium-os/developer-library/guides/portage/subtools-builder/"

LICENSE="metapackage"
SLOT="0"
KEYWORDS="*"

RDEPEND="
	app-arch/pixz-subtool
	app-arch/zstd-subtool
	app-emulation/qemu-subtool
	app-emulation/renode
	chromeos-base/update_engine
	chromeos-base/vboot_reference
	chromeos-base/verity
	dev-util/rustfmt-subtool
	dev-util/shellcheck
	dev-util/unifdef-subtool
	sys-apps/coreboot-utils
	sys-apps/pv-subtool
	sys-devel/aapt
	sys-devel/zipalign
	sys-fs/squashfs-tools
"

# Disable default install rules (e.g. docs).
src_install() { :; }
