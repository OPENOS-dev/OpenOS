# Copyright 2018 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_WORKON_COMMIT="e67643c64a105f6f744b007eb857f381ace07e8e"
CROS_WORKON_TREE=("518b50f8b6d01e95cbd933487ed7c6452ac4acb3" "aa3f87c5c42bbab1bc293a3491f7ff4d2df1f1e7" "f91b6afd5f2ae04ee9a2c19109a3a4a36f7659e6")
CROS_WORKON_INCREMENTAL_BUILD="1"
CROS_WORKON_LOCALNAME="platform2"
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_OUTOFTREE_BUILD=1
CROS_WORKON_SUBTREE="common-mk arc/container/bundle .gn"

inherit cros-workon user

DESCRIPTION="Container to run Android."
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform2/+/HEAD/arc/container/bundle"

LICENSE="BSD-Google"
KEYWORDS="*"

IUSE="
	arcpp
	arcvm
	"

REQUIRED_USE="|| ( arcpp arcvm )"

RDEPEND="!<chromeos-base/chromeos-cheets-scripts-0.0.3"
DEPEND="${RDEPEND}"

CONTAINER_ROOTFS="/opt/google/containers/android/rootfs"

src_install() {
	if use arcpp; then
		insinto /opt/google/containers/android
		doins arc/container/bundle/rvc/config.json

		insinto /lib/udev/rules.d
		doins arc/container/bundle/rvc/95-arcinput.rules

		# Install exception file for FIFO blocking policy on stateful partition.
		insinto /usr/share/cros/startup/fifo_exceptions
		doins arc/container/bundle/arc-fifo-exceptions.txt

		# Install exception file for symlink blocking policy on stateful partition.
		insinto /usr/share/cros/startup/symlink_exceptions
		doins arc/container/bundle/arc-symlink-exceptions.txt
	fi
}

pkg_preinst() {
	# ARCVM also needs these users on the host side for proper ugid remapping.
	enewuser "wayland"
	enewgroup "wayland"
	enewuser "arc-bridge"
	enewgroup "arc-bridge"
	enewuser "android-root"
	enewgroup "android-root"
	enewgroup "arc-sensor"
	enewgroup "android-everybody"
	enewgroup "android-reserved-disk"
}

pkg_postinst() {
	if use arcpp; then
		local root_uid=$(egetent passwd android-root | cut -d: -f3)
		local root_gid=$(egetent group android-root | cut -d: -f3)

		# Create a rootfs directory, and then a subdirectory mount point. We
		# use 0500 for CONTAINER_ROOTFS instead of 0555 so that non-system
		# processes running outside the container don't start depending on
		# files in system.raw.img.
		# These are created here rather than at
		# install because some of them may already exist and have mounts.
		install -d --mode=0500 "--owner=${root_uid}" "--group=${root_gid}" \
			"${ROOT}${CONTAINER_ROOTFS}" \
			|| true
		# This CONTAINER_ROOTFS/root directory works as a mount point for
		# system.raw.img, and once it's mounted, the image's root directory's
		# permissions override the mode, owner, and group mkdir sets here.
		mkdir -p "${ROOT}${CONTAINER_ROOTFS}/root" || true
		install -d --mode=0500 "--owner=${root_uid}" "--group=${root_gid}" \
			"${ROOT}${CONTAINER_ROOTFS}/android-data" \
			|| true
	fi
}
