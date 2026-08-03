# Copyright 2014 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI="7"

# This ebuild only cares about its own FILESDIR and ebuild file, so it tracks
# the canonical empty project.
CROS_WORKON_COMMIT="d2d95e8af89939f893b1443135497c1f5572aebc"
CROS_WORKON_TREE="776139a53bc86333de8672a51ed7879e75909ac9"
CROS_WORKON_PROJECT="chromiumos/infra/build/empty-project"
CROS_WORKON_LOCALNAME="../platform/empty-project"
CROS_WORKON_OUTOFTREE_BUILD=1

PYTHON_COMPAT=( python3_{8..12} )

inherit cros-workon python-r1

DESCRIPTION="List of packages that are needed inside the ChromiumOS dev image"
HOMEPAGE="https://dev.chromium.org/"

LICENSE="metapackage"
SLOT="0"
KEYWORDS="*"
# Note: Do not utilize USE=internal here.  Update virtual/target-chrome-os-dev.
IUSE="
	asan
	biod
	cellular
	chromeless_tty
	cras
	cros_embedded
	diag
	dlc
	hps
	input_devices_spi_heatmap
	lldbserver
	nvme
	opengl
	pam
	+power_management
	+profile
	+shill
	scx
	tpm
	tpm2
	ubsan
	ufs
	usb
	vaapi
	video_cards_amdgpu
	video_cards_intel
	video_cards_mediatek
	video_cards_msm
	vulkan
"
REQUIRED_USE="${PYTHON_REQUIRED_USE}"

# The dependencies here are meant to capture "all the packages
# developers want to use for development, test, or debug".  This
# category is meant to include all developer use cases, including
# software test and debug, performance tuning, hardware validation,
# and debugging failures running autotest.
#
# To protect developer images from changes in other ebuilds you
# should include any package with a user constituency, regardless of
# whether that package is included in the base ChromiumOS image or
# any other ebuild.
#
# Don't include packages that are indirect dependencies: only
# include a package if a file *in that package* is expected to be
# useful.

################################################################################
#
# CROS_* : Dependencies for CrOS devices (coreutils, etc.)
#
################################################################################
CROS_X86_RDEPEND="
	power_management? ( dev-util/turbostat )
	sys-apps/dmidecode
	sys-apps/pciutils
	sys-boot/syslinux
	vaapi? (
		chromeos-base/libva-fake-driver
		media-gfx/vadumpcaps
		media-video/libva-utils
	)
"

RDEPEND="
	${PYTHON_DEPS}
	x86? ( ${CROS_X86_RDEPEND} )
	amd64? ( ${CROS_X86_RDEPEND} )
"

RDEPEND="${RDEPEND}
	pam? ( app-admin/sudo )
	app-admin/sysstat
	app-arch/bzip2
	app-arch/gzip
	app-arch/tar
	app-arch/unzip
	app-arch/xz-utils
	app-arch/zip
	biod? ( chromeos-base/biod-dev )
	profile? (
		chromeos-base/quipper
		net-analyzer/netperf
		dev-util/perf
	)
	app-benchmarks/stress-ng
	app-crypt/nss
	tpm? ( app-crypt/tpm-tools )
	app-editors/nano
	app-editors/vim
	app-misc/edid-decode
	app-misc/evtest
	app-misc/pax-utils
	app-misc/screen
	app-portage/portage-utils
	app-shells/bash
	app-text/tree
	cras? (
		chromeos-base/audiotest
		media-sound/sox
	)
	chromeos-base/avtest_label_detect
	chromeos-base/chromeos-dev-root
	!cros_embedded? ( chromeos-base/cryptohome-dev-utils )
	dlc? (
		chromeos-base/dlcservice-dev
	)
	biod? ( chromeos-base/ec-npcx-monitor )
	shill? ( chromeos-base/ethernet-hide )
	tpm2? ( chromeos-base/g2f_tools )
	!chromeless_tty? ( chromeos-base/graphics-utils-go )
	input_devices_spi_heatmap? ( chromeos-base/heatmap-recorder )
	hps? (
		chromeos-base/hps-tool
	)
	chromeos-base/hwid_extractor
	cellular? ( chromeos-base/modemloggerd-dev )
	chromeos-base/mctk
	chromeos-base/policy_utils
	chromeos-base/pp_cli
	chromeos-base/protofiles
	!chromeless_tty? ( chromeos-base/screen-capture-utils www-apps/novnc )
	shill? ( chromeos-base/shill-test-scripts )
	chromeos-base/touch_firmware_test
	chromeos-base/usi-test
	dev-vcs/git
	net-analyzer/tcpdump
	net-analyzer/speedtest-cli
	net-analyzer/traceroute
	net-dialup/minicom
	net-dns/bind-tools
	net-misc/dhcp
	diag? ( net-misc/diag )
	net-misc/iperf:2
	net-misc/iputils
	net-misc/openssh
	net-misc/qlog
	net-misc/rsync
	net-wireless/iw
	net-wireless/wireless-tools
	dev-libs/libgpiod
	dev-python/protobuf-python
	dev-python/cherrypy
	dev-python/dbus-python
	dev-python/pydbus
	dev-python/hid-tools
	dev-util/drm_info
	dev-util/hdctools
	lldbserver? ( dev-util/lldb-server )
	dev-util/mem
	dev-debug/strace
	media-libs/libv4l
	media-libs/libyuv-test
	media-libs/openh264
	vulkan? (
		dev-util/vulkan-tools
		media-libs/vulkan-layers
	)
	media-video/yavta
	net-dialup/lrzsz
	net-fs/sshfs
	net-misc/curl
	net-misc/wget
	sys-apps/coreboot-utils
	sys-apps/coreutils
	sys-apps/diffutils
	sys-apps/file
	sys-apps/findutils
	sys-apps/flashrom-tester
	sys-apps/gawk
	sys-apps/i2c-tools
	sys-apps/iotools
	sys-apps/kexec-lite
	sys-apps/less
	sys-apps/mmc-utils
	nvme? ( sys-apps/nvme-cli )
	sys-apps/portage
	scx? (
		sys-apps/scx
	)
	sys-apps/smartmontools
	ufs? (
		sys-apps/sg3_utils
		sys-apps/ufs-utils
	)
	usb? ( sys-apps/usbutils )
	sys-apps/which
	sys-block/fio
	sys-devel/binutils
	sys-devel/gdb
	sys-fs/cryptsetup
	sys-fs/fuse
	sys-fs/lvm2
	sys-fs/quota
	power_management? ( sys-power/powertop )
	sys-process/procps
	sys-process/psmisc
	sys-process/time
	sys-process/usbtop
	virtual/autotest-capability
	virtual/chromeos-bsp-dev
	video_cards_amdgpu? ( x11-apps/igt-gpu-tools )
	video_cards_intel? ( x11-apps/igt-gpu-tools )
	video_cards_mediatek? ( x11-apps/igt-gpu-tools )
	video_cards_msm? ( x11-apps/igt-gpu-tools )
"
