# Copyright 1999-2022 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_WORKON_PROJECT="chromiumos/infra/build/empty-project"
CROS_WORKON_LOCALNAME="../platform/empty-project"
CROS_WORKON_OUTOFTREE_BUILD=1

inherit cros-workon

# cros-workon's 9999 doesn't play nicely with the other version detection in
# here.
MY_PV="5.4"

# shellcheck disable=SC2034
ETYPE="headers"
# shellcheck disable=SC2034
H_SUPPORTEDARCH="alpha amd64 arc arm arm64 avr32 cris frv hexagon hppa ia64 m68k metag microblaze mips mn10300 nios2 openrisc ppc ppc64 riscv s390 score sh sparc x86 xtensa"
inherit kernel-2
# shellcheck disable=SC2034
CKV="${MY_PV}"
detect_version

PATCH_VER="2"
SRC_URI="${KERNEL_URI}
	${PATCH_VER:+mirror://gentoo/gentoo-headers-${MY_PV}-${PATCH_VER}.tar.xz}
	${PATCH_VER:+https://dev.gentoo.org/~sam/distfiles/gentoo-headers-${MY_PV}-${PATCH_VER}.tar.xz}
"
S="${WORKDIR}/linux-${MY_PV}"

KEYWORDS="~*"

BDEPEND="
	app-arch/xz-utils
	dev-lang/perl
	net-misc/rsync"

[[ -n ${PATCH_VER} ]] && PATCHES=("${WORKDIR}/${MY_PV}")

#
# NOTE: All the patches must be applicable using patch -p1.
#
PATCHES+=(
	"${FILESDIR}/0001-BACKPORT-sync-nl80211.h-to-v5.8.patch"
	"${FILESDIR}/0002-BACKPORT-fscrypt-add-support-for-IV_INO_LBLK_64-poli.patch"
	"${FILESDIR}/0003-FROMGIT-Input-add-privacy-screen-toggle-keycode.patch"
	"${FILESDIR}/0004-BACKPORT-Input-Add-FULL_SCREEN-ASPECT_RATIO-SELECTIV.patch"
	"${FILESDIR}/0005-BACKPORT-vfs-add-faccessat2-syscall.patch"
	"${FILESDIR}/0006-BACKPORT-add-close_range-syscall-definitions.patch"
	"${FILESDIR}/0007-CHROMIUM-Add-dma-heap-header.patch"
	"${FILESDIR}/0008-UPSTREAM-vsock-add-VMADDR_CID_LOCAL-definition.patch"
	"${FILESDIR}/0009-BACKPORT-UPSTREAM-rtnetlink-provide-permanent-hardwa.patch"
	# The following patch was introduced in v5.5 and later applied to v5.4.12;
	# we need to backport it as we use the v5.4.0 tarball to generate headers
	"${FILESDIR}/0010-BACKPORT-UPSTREAM-arm64-Move-__ARCH_WANT_SYS_CLONE3-.patch"
	# Above patches are from before and up to v5.10
	# (Note: the first 0011 patch was added later.)
	"${FILESDIR}/0011-BACKPORT-backport-epoll_pwait2.patch"
	"${FILESDIR}/0011-FROMLIST-media-rkisp1-Add-user-space-ABI-definitions.patch"
	"${FILESDIR}/0011-BACKPORT-HID-hidraw-Add-additional-hidraw-input-output-report-ioctls.patch"
	# Above patches are from before and up to v5.15
	"${FILESDIR}/0012-BACKPORT-LoadPin-Enable-loading-from-trusted-dm-veri.patch"
	# Above patches are from before and up to v6.1
	"${FILESDIR}/0013-BACKPORT-mseal-wire-up-mseal-syscall.patch"
	# Above patches are from before and up to v6.12

	# The following patches were never merged upstream
	"${FILESDIR}/1001-videodev2.h-add-IPU3-meta-buffer-format.patch"
	"${FILESDIR}/1002-uapi-intel-ipu3-Add-user-space-ABI-definitions.patch"
	"${FILESDIR}/1003-virtwl-add-virtwl-driver.patch"
	"${FILESDIR}/1004-FROMLIST-media-pixfmt-Add-Mediatek-ISP-P1-image-meta.patch"
	"${FILESDIR}/1005-CHROMIUM-linux-headers-update-headers-with-UVC-1.5-R.patch"
	"${FILESDIR}/1006-CHROMIUM-v4l2-controls-use-very-high-ID-for-ROI-auto.patch"
	"${FILESDIR}/1007-ASoC-SOF-Add-userspace-ABI-support.patch"
	"${FILESDIR}/1008-BACKPORT-FROMLIST-media-uvcvideo-implement-UVC-v1.5-.patch"
	"${FILESDIR}/1009-CHROMIUM-media-uvcvideo-support-roi-coordinate-syste.patch"
)

# This list contains all V4L2 patches backported from upstream, along with
# two downstream patches that are V4L2-related, need to be applied after
# the V4L2 bunch, and scheduled to be removed.
PATCHES+=(
	"${FILESDIR}/v4l2/0001-BACKPORT-media-vb2-add-V4L2_BUF_FLAG_M2M_HOLD_CAPTUR.patch"
	"${FILESDIR}/v4l2/0002-BACKPORT-media-videodev2.h-add-V4L2_DEC_CMD_FLUSH.patch"
	"${FILESDIR}/v4l2/0003-BACKPORT-media-videobuf2-add-V4L2_FLAG_MEMORY_NON_CO.patch"
	"${FILESDIR}/v4l2/0004-BACKPORT-media-videobuf2-handle-V4L2_FLAG_MEMORY_NON.patch"
	"${FILESDIR}/v4l2/0005-BACKPORT-media-media-v4l2-remove-V4L2_FLAG_MEMORY_NO.patch"
	"${FILESDIR}/v4l2/0006-BACKPORT-media-v4l2-ctrl-Add-VP9-codec-levels.patch"
	"${FILESDIR}/v4l2/0007-BACKPORT-media-v4l2-ctrl-Add-H264-profile-and-levels.patch"
	# Above patches are from before and up to v5.10
	"${FILESDIR}/v4l2/0011-BACKPORT-media-videodev2.h-v4l2-ioctl-add-rkisp1-met.patch"
	"${FILESDIR}/v4l2/0012-BACKPORT-media-Rename-stateful-codec-control-macros.patch"
	"${FILESDIR}/v4l2/0013-BACKPORT-media-controls-Add-the-stateless-codec-cont.patch"
	"${FILESDIR}/v4l2/0014-BACKPORT-media-uapi-Move-parsed-H264-pixel-format-ou.patch"
	"${FILESDIR}/v4l2/0015-BACKPORT-media-uapi-Move-the-H264-stateless-control-.patch"
	"${FILESDIR}/v4l2/0016-BACKPORT-media-uapi-move-H264-stateless-controls-out.patch"
	"${FILESDIR}/v4l2/0017-BACKPORT-media-uapi-Move-parsed-VP8-pixel-format-out.patch"
	"${FILESDIR}/v4l2/0018-BACKPORT-media-uapi-Move-the-VP8-stateless-control-t.patch"
	"${FILESDIR}/v4l2/0019-BACKPORT-media-uapi-move-VP8-stateless-controls-out-.patch"
	"${FILESDIR}/v4l2/0020-BACKPORT-media-v4l2-ctrl-Add-layer-wise-bitrate-cont.patch"
	"${FILESDIR}/v4l2/0021-BACKPORT-media-vicodec-mark-the-stateless-FWHT-API-as-stable.patch"
	"${FILESDIR}/v4l2/0022-BACKPORT-media-v4l-Add-new-Colorimetry-Class.patch"
	"${FILESDIR}/v4l2/0023-BACKPORT-media-v4l2-ctrls-Add-encoder-constant-quality-contro.patch"
	# Above patches are from before and up to v5.15
	"${FILESDIR}/v4l2/0031-UPSTREAM-media-add-Mediatek-s-MM21-format.patch"
	"${FILESDIR}/v4l2/0032-BACKPORT-media-videobuf2-add-V4L2_MEMORY_FLAG_NON_CO.patch"
	"${FILESDIR}/v4l2/0033-BACKPORT-media-videobuf2-handle-V4L2_MEMORY_FLAG_NON.patch"
	"${FILESDIR}/v4l2/0034-BACKPORT-media-uapi-Add-VP9-stateless-decoder-contro.patch"
	"${FILESDIR}/v4l2/0035-BACKPORT-media-Add-P010-video-format.patch"
	"${FILESDIR}/v4l2/0036-UPSTREAM-media-videodev2.h-add-V4L2_CTRL_FLAG_DYNAMI.patch"
	"${FILESDIR}/v4l2/0037-BACKPORT-media-uapi-Move-parsed-HEVC-pixel-format-ou.patch"
	"${FILESDIR}/v4l2/0038-BACKPORT-media-uapi-Move-the-HEVC-stateless-control-.patch"
	"${FILESDIR}/v4l2/0039-BACKPORT-media-uapi-move-HEVC-stateless-controls-out.patch"
	# Above patches from before and up to v6.1
	"${FILESDIR}/v4l2/0051-BACKPORT-media-add-Sorenson-Spark-video-format.patch"
	"${FILESDIR}/v4l2/0052-BACKPORT-media-add-RealVideo-format-RV30-and-RV40.patch"
	"${FILESDIR}/v4l2/0053-BACKPORT-media-uapi-HEVC-Add-num_delta_pocs_of_ref_r.patch"
	"${FILESDIR}/v4l2/0054-BACKPORT-media-Add-AV1-uAPI.patch"
	"${FILESDIR}/v4l2/0055-BACKPORT-FROMLIST-media-v4l2_ctrl-Add-V4L2_CTRL_TYPE.patch"
	"${FILESDIR}/v4l2/0056-BACKPORT-FROMLIST-v4l2-ctrls-add-support-for-V4L2_CT.patch"
	"${FILESDIR}/v4l2/0057-BACKPORT-FROMLIST-media-uvcvideo-implement-UVC-v1.5-.patch"
	# Above patches are from after v6.1

	# This is the end of the list. Please add new entries above this. Entries
	# below are expected to be removed soon.

	# Empty placeholder files for old *-ctrls-upstream.h header files
	# TODO (b/278157861) remove after header migration and inclusion removed
	# from Chromium
	"${FILESDIR}/v4l2/9998-CHROMIUM-v4l-Add-placeholder-header-files-for-split-.patch"
)

src_unpack() {
	# avoid kernel-2_src_unpack
	default
}

src_prepare() {
	# avoid kernel-2_src_prepare
	default
}

src_install() {
	kernel-2_src_install

	find "${ED}" \( -name '.install' -o -name '*.cmd' \) -delete || die
	# delete empty directories
	find "${ED}" -empty -type d -delete || die
}

src_test() {
	# Make sure no uapi/ include paths are used by accident.
	grep -E -r \
		-e '# *include.*["<]uapi/' \
		"${D}" && die "#include uapi/xxx detected"

	einfo "Possible unescaped attribute/type usage"
	grep -E -r \
		-e '(^|[[:space:](])(asm|volatile|inline)[[:space:](]' \
		-e '\<([us](8|16|32|64))\>' \
		.

	einfo "Missing linux/types.h include"
	grep -E -l -r -e '__[us](8|16|32|64)' "${ED}" | xargs grep -L linux/types.h

	emake ARCH="$(tc-arch-kernel)" LD="$(tc-getLD)" headers_check
}
