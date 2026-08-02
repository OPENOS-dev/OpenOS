# Copyright 2021 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI="7"

DESCRIPTION="Ebuild to support the Chrome OS TI50 device."

LICENSE="BSD-Google Apache-2.0 MIT"
SLOT="0"
KEYWORDS="*"
IUSE="cros_host"

# CR50 and TI50 share the same development tools, e.g. gsctool
RDEPEND="chromeos-base/chromeos-cr50-dev
	!cros_host? (
		chromeos-base/chromeos-cr50-scripts
		chromeos-base/hwsec-utils
	)"

# There are two major types of images of Ti50, prod (used on most MP devices)
# and pre-pvt, used on devices still not fully released.
DT_PROD_IMAGE="ti50.r0.0.62.w0.23.290"
DT_PRE_PVT_IMAGE="ti50.r0.0.62.w0.24.290_FFFF_00000000_00000010"
NT_PROD_IMAGE="ti50-nt.r2.0.119.w0.33.291"
NT_PRE_PVT_IMAGE="ti50-nt.r2.0.119.w0.34.291_FFFF_00000000_00000010"

# Ensure all images and included in the manifest.
TI50_BASE_NAMES=( \
	"${DT_PROD_IMAGE}" \
	"${DT_PRE_PVT_IMAGE}" \
	"${NT_PROD_IMAGE}" \
	"${NT_PRE_PVT_IMAGE}" )
MIRROR_PATH="gs://chromeos-localmirror/distfiles/"
SRC_URI="$(printf " ${MIRROR_PATH}/%s.tar.xz" "${TI50_BASE_NAMES[@]}")"

S="${WORKDIR}"

src_install() {
	# Always install both pre-pvt and MP Ti50 images, let the updater at
	# run time decide which one to use, based on the GSC Board ID flags
	# value.

	insinto /opt/google/ti50/firmware

	einfo "Will install ${DT_PROD_IMAGE}, ${DT_PRE_PVT_IMAGE}, "\
			"${NT_PROD_IMAGE}, and ${NT_PRE_PVT_IMAGE}."

	# TODO(b/371037062): Remove these two legacy paths once all scripts
	# have updated.
	newins "${DT_PROD_IMAGE}"/*.bin.prod ti50.bin.prod
	newins "${DT_PRE_PVT_IMAGE}"/*.bin.prod ti50.bin.prepvt

	newins "${DT_PROD_IMAGE}"/*.bin.prod ti50-dt.bin.prod
	newins "${DT_PRE_PVT_IMAGE}"/*.bin.prod ti50-dt.bin.prepvt

	newins "${NT_PROD_IMAGE}"/*.bin.prod ti50-nt.bin.prod
	newins "${NT_PRE_PVT_IMAGE}"/*.bin.prod ti50-nt.bin.prepvt
}
