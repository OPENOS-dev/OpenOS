# Copyright 2020 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

DESCRIPTION="Initialization files for Quickoffice on Chrome OS"
HOMEPAGE="https://www.chromium.org"
# To update, copy from chrome-quickoffice bucket to chromeos-localmirror,
# Rename this ebuild file to match the new version, and update the Manifest.
# gsutil cp -a public-read gs://chrome-quickoffice/waterfallv2/releases/nacl/147.638/147.638.665/bundle_app/quickoffice-chrome-147.638.665_compExt.zip gs://chromeos-localmirror/distfiles/
# ebuild ../third_party/chromiumos-overlay/chromeos-base/quickoffice/quickoffice-*.ebuild manifest
SRC_URI="gs://chrome-quickoffice/waterfallv2/releases/nacl/${PV%.*}/${PV}/bundle_app/quickoffice-chrome-${PV}_compExt.zip"

LICENSE="BSD-Google"
SLOT="0"
KEYWORDS="*"
S="${WORKDIR}"

BDEPEND="
	app-arch/unzip
	sys-fs/squashfs-tools
"

src_compile() {
	# Delete unneeded files.
	rm -rf templates upstart

	# Compress all of quickoffice to save space on the rootfs.
	# - compress with LZO and 1M blocks to optimize trade-off between
	# compression ratio and decompression speed.
	# - use "-keep-as-directory" option so the squash file will include the
	# folder with the name of the CPU architecture, which is expected by the
	# scripts on device.
	# - use "-root-mode 0755" to ensure that the mountpoint has permissions
	# 0755 instead of the default 0777.
	# - use "-4k-align" option so individual files inside the squash file
	# will be aligned to 4K blocks, which improves the efficiency of the
	# delta updates.
	mksquashfs ./* quickoffice.squash \
		-all-root -noappend -no-recovery -no-exports \
		-exit-on-error -comp lzo -b 1M -keep-as-directory \
		-4k-align -root-mode 0755 -no-progress \
		|| die "Failed to create Quickoffice squashfs"
}

src_install() {
	insinto /usr/share/chromeos-assets
	doins quickoffice.squash
	# Create the directory where the Quickoffice squashfs will be mounted.
	keepdir /usr/share/chromeos-assets/quickoffice

	# Upstart script that will automatically mount/unmount the Quickoffice
	# squashfs when the device starts/stops.
	insinto /etc/init
	doins "${FILESDIR}/quickoffice.conf"
}
