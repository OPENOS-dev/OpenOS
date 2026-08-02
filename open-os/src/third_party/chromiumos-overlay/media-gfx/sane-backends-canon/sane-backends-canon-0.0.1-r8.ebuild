EAPI=7

CROS_WORKON_COMMIT="997068caa228cd38c08fce1ba91b83b0d4f17d8f"
CROS_WORKON_TREE="eb9f40d3f632489098a61db4717e9ab33a0f44a9"
inherit cros-constants

CROS_WORKON_PROJECT="chromiumos/third_party/canon-sane-backend"
CROS_WORKON_LOCALNAME="../third_party/canon-sane-backend"

inherit dlc cros-workon flag-o-matic udev

DESCRIPTION="Canon SANE API Scanner Driver"
HOMEPAGE="https://www.canon.com"
LICENSE="Apache-2.0"
KEYWORDS="*"

# Original DLC Size = 1691648
# ceil(1691648 * 1.3) = 2199143
# ceil(2199143 / 4096) = 537 PREALLOC blocks
DLC_PREALLOC_BLOCKS="537"
DLC_SCALED=true

DEPEND="
	virtual/libusb:1
	media-libs/libjpeg-turbo:=
	media-gfx/sane-backends:=
"

RDEPEND="
	${DEPEND}
"
src_compile() {

	pushd "${S}/source" || die

	export LIBDIR="${ESYSROOT}/usr/$(get_libdir)"
	tc-export CXX
	tc-export CC
	tc-export AR
	tc-export RANLIB
	# From b/201531268
	append-lfs-flags

	# The below build scripts are commented out here for documentation purposes.
	# This script was provided to change permissions w/ chmod, we set
	# the build script permissions in source instead.
	# ./1_run_first.sh
	# This script was provided to build 3p libs, we want to use our
	# SYSROOT libs instead.
	# ./2_install_libs.sh

	# Provided script that drives the ChromeOS build.
	./3_chromeos_sane.sh || die

	popd || die

}

src_install() {

	insinto  /lib/udev/rules.d
	doins "${S}"/source/files/41-canon-sane.rules

	insinto "$(dlc_add_path /canonlibs)"
	doins "${S}"/source/files/CeiUSBLinux.so
	doins "${S}"/source/files/CsdCore.so

	doins "${S}"/source/files/libsane-drm260.so.1
	doins "${S}"/source/files/drm260vs.so
	doins "${S}"/source/files/drm260.ini
	doins "${S}"/source/files/drm260.sane.ini

	doins "${S}"/source/files/libsane-p208ii.so.1
	doins "${S}"/source/files/p208iivs.so
	doins "${S}"/source/files/p208ii.ini
	doins "${S}"/source/files/p208ii.sane.ini

	doins "${S}"/source/files/libsane-drp208ii.so.1
	doins "${S}"/source/files/drp208iivs.so
	doins "${S}"/source/files/drp208ii.ini
	doins "${S}"/source/files/drp208ii.sane.ini

	doins "${S}"/source/files/libsane-p215ii.so.1
	doins "${S}"/source/files/p215iivs.so
	doins "${S}"/source/files/p215ii.ini
	doins "${S}"/source/files/p215ii.sane.ini

	doins "${S}"/source/files/libsane-drp215ii.so.1
	doins "${S}"/source/files/drp215iivs.so
	doins "${S}"/source/files/drp215ii.ini
	doins "${S}"/source/files/drp215ii.sane.ini

	doins "${S}"/source/files/libsane-drc225ii.so.1
	doins "${S}"/source/files/drc225iivs.so
	doins "${S}"/source/files/drc225ii.ini
	doins "${S}"/source/files/drc225ii.sane.ini

	doins "${S}"/source/files/libsane-drc230.so.1
	doins "${S}"/source/files/drc230vs.so
	doins "${S}"/source/files/drc230.ini
	doins "${S}"/source/files/drc230.sane.ini

	doins "${S}"/source/files/libsane-drc240.so.1
	doins "${S}"/source/files/drc240vs.so
	doins "${S}"/source/files/drc240.ini
	doins "${S}"/source/files/drc240.sane.ini

	doins "${S}"/source/files/libsane-r40.so.1
	doins "${S}"/source/files/r40vs.so
	doins "${S}"/source/files/r40.ini
	doins "${S}"/source/files/r40.sane.ini

	doins "${S}"/source/files/libsane-r50.so.1
	doins "${S}"/source/files/r50vs.so
	doins "${S}"/source/files/r50.ini
	doins "${S}"/source/files/r50.sane.ini

	## Add configuration - we do *NOT* DLC these because these are small files ##
	# We also need this in rootfs so that lorgnette can both install and
	# then subsequently recognize the DLC driver during the same
	# discover session.
	# Add per-device configuration
	insinto /etc/sane.d
	doins "${S}"/source/files/canon.conf

	insinto /etc/sane.d/dll.d
	doins "${S}"/source/files/canon

	dlc_src_install
}
