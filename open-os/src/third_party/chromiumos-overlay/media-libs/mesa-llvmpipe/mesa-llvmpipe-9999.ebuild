# Copyright 1999-2024 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_WORKON_PROJECT="chromiumos/third_party/mesa"
CROS_WORKON_MANUAL_UPREV="1"
CROS_WORKON_LOCALNAME="mesa"
CROS_WORKON_EGIT_BRANCH="upstream/main"

PYTHON_COMPAT=( python3_11 )
inherit meson cros-workon python-any-r1

DESCRIPTION="OpenGL-like graphic library for Linux"
HOMEPAGE="https://www.mesa3d.org/ https://mesa.freedesktop.org/"

LICENSE="MIT SGI-B-2.0"
SLOT="0"
KEYWORDS="~*"

IUSE="debug egl gles2 kvm_guest libglvnd selinux video_cards_virgl vulkan"

RDEPEND="
	!media-libs/mesa

	libglvnd? ( media-libs/libglvnd )
	!libglvnd? ( !media-libs/libglvnd )

	vulkan? (
		app-arch/zstd:=
		virtual/libudev:=
	)

	dev-libs/expat
	>=sys-libs/zlib-1.2.9
	>=x11-libs/libdrm-2.4.109
"
DEPEND="
	${RDEPEND}
	sys-devel/llvm:15=
"
BDEPEND="
	sys-devel/bison
	sys-devel/flex
	virtual/pkgconfig
	$(python_gen_any_dep '
		dev-python/mako[${PYTHON_USEDEP}]
		dev-python/packaging[${PYTHON_USEDEP}]
		dev-python/pyyaml[${PYTHON_USEDEP}]
	')
"

python_check_deps() {
	python_has_version -b \
		"dev-python/mako[${PYTHON_USEDEP}]" \
		"dev-python/packaging[${PYTHON_USEDEP}]" \
		"dev-python/pyyaml[${PYTHON_USEDEP}]"
}

pkg_setup() {
	python-any-r1_pkg_setup
	cros-workon_pkg_setup
}

src_configure() {
	cros_optimize_package_for_speed

	export LLVM_CONFIG=${SYSROOT}/usr/lib/llvm/bin/llvm-config-host

	local gallium_drivers=llvmpipe
	use video_cards_virgl && gallium_drivers+=",virgl"

	local emesonargs=(
		--buildtype $(usex debug debug plain)
		-Db_ndebug=$(usex debug false true)

		-Dgbm=disabled
		-Dgles1=disabled
		-Dglx=disabled
		-Dshader-cache=disabled
		-Dzstd=disabled
		-Dunversion-libgallium=true

		-Dplatforms=""
		-Degl-native-platform="surfaceless"

		-Dllvm=enabled
		-Dshared-llvm=disabled

		-Dgallium-drivers="${gallium_drivers}"
		-Dvulkan-drivers=$(usex vulkan swrast '')

		$(meson_feature egl)
		$(meson_feature gles2)
		$(meson_feature libglvnd glvnd)
		$(meson_use selinux)
	)

	if use kvm_guest; then
		emesonargs+=( -Ddri-search-path=/opt/google/cros-containers/lib )
	fi

	meson_src_configure
}

src_install() {
	meson_src_install

	# Remove redundant GLES headers
	rm -f "${D}"/usr/include/{EGL,GLES2,GLES3,KHR}/*.h || die "Removing GLES headers failed."

	insinto "/usr/share/drirc.d"
	insopts -m0644
	doins "${FILESDIR}"/01-vkms.conf
}
