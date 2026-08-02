# Copyright 2017 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

# This ebuild only cares about its own FILESDIR and ebuild file, so it tracks
# the canonical empty project.
CROS_WORKON_COMMIT="d2d95e8af89939f893b1443135497c1f5572aebc"
CROS_WORKON_TREE="776139a53bc86333de8672a51ed7879e75909ac9"
CROS_WORKON_PROJECT="chromiumos/infra/build/empty-project"
CROS_WORKON_LOCALNAME="platform/empty-project"
CROS_WORKON_OUTOFTREE_BUILD=1

inherit cros-workon udev user

DESCRIPTION="Ebuild to support the Chrome OS Cr50 device."

LICENSE="BSD-Google"
SLOT="0"
KEYWORDS="*"
IUSE="generic_tpm2 cr50_onboard ti50_onboard cr50_disable_sleep_in_suspend"

DEPEND=""

RDEPEND="
	chromeos-base/ec-utils
	chromeos-base/vboot_reference:=
	!<chromeos-base/chromeos-cr50-0.0.1-r38
"

pkg_preinst() {
	enewuser "rma_fw_keeper"
	enewgroup "rma_fw_keeper"
	enewgroup "suzy-q"
}

src_install() {
	local files
	local f

	insinto /etc/init
	files=(
		cr50-metrics.conf
		cr50-result.conf
		cr50-update.conf
	)
	for f in "${files[@]}"; do
		doins "${FILESDIR}/${f}"
	done

	if use cr50_disable_sleep_in_suspend; then
		doins "${FILESDIR}/cr50-disable-sleep.conf"
	fi
	if use cr50_onboard; then
		doins "${FILESDIR}/cr50-disable-strongbox.conf"
	fi

	udev_dorules "${FILESDIR}"/99-cr50.rules

	exeinto /usr/share/cros

	# TODO(b/289003370): The gsc-constants.sh is referenced in multiple
	# locations in the factory related flow.
	if use ti50_onboard; then
		f="ti50-constants.sh"
	elif use cr50_onboard || use generic_tpm2; then
		f="cr50-constants.sh"
	else
		die "Neither GSC nor generic TPM2 is used"
	fi
	newexe "${FILESDIR}/${f}" "gsc-constants.sh"

	insinto /opt/google/cr50/ro_db
	doins "${FILESDIR}"/ro_db/*.db
}
