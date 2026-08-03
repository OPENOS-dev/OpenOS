# Copyright 2023 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

PYTHON_COMPAT=( python3_{8..11} )
inherit meson cros-toolchain-funcs python-any-r1

# intel_clc is an executable that is used to compile OpenCL C code to
# SPIR-V during the build of media-libs/mesa-iris. It is needed to build
# code to support Vulkan ray tracing APIs on Intel MTL GPUs.
#
# intel_clc is a part of Mesa, but since it must run on the build machine,
# it was split it into its own package that gets upreved independently."
DESCRIPTION="intel_clc tool used for building OpenCL C to SPIR-V"
HOMEPAGE="https://mesa3d.org/"

MY_PV="${PV/_/-}"
SRC_URI="https://archive.mesa3d.org/mesa-${MY_PV}.tar.xz"

LICENSE="MIT SGI-B-2.0"
SLOT="$(ver_cut 1-2)"
KEYWORDS="*"
IUSE="debug"

# N.B., libclc-16* isn't _strictly_ required, but crrev.com/c/4998677
# "downgraded" libclc. The stringent version requirement here ensures that
# Portage does the right thing rather than allowing old binpkgs.
DEPEND="
	app-arch/zstd:=
	=dev-libs/libclc-16*
	dev-libs/expat:=
	dev-libs/libxml2:=
	dev-util/spirv-tools:=
	>=sys-libs/zlib-1.2.8:=
"
RDEPEND="${DEPEND}"
BDEPEND="
	dev-util/spirv-llvm:=
	$(python_gen_any_dep '
		dev-python/mako[${PYTHON_USEDEP}]
		dev-python/packaging[${PYTHON_USEDEP}]
		dev-python/pyyaml[${PYTHON_USEDEP}]
	')
"

S="${WORKDIR}/mesa-${MY_PV}"

python_check_deps() {
	python_has_version -b "dev-python/mako[${PYTHON_USEDEP}]" &&
	python_has_version -b "dev-python/packaging[${PYTHON_USEDEP}]" &&
	python_has_version -b "dev-python/pyyaml[${PYTHON_USEDEP}]"
}

src_prepare() {
	default

	# Disable automatic dependency on libdrm
	sed -i -e 's/system_has_kms_drm = .*/system_has_kms_drm = false/g' meson.build || die
}

src_configure() {
	tc-export PKG_CONFIG

	# Configure this to use spirv-llvm's llvm artifacts
	local spirv_llvm="/opt/spirv-llvm"
	export PKG_CONFIG_PATH="${spirv_llvm}/lib/pkgconfig"
	# shellcheck disable=SC2034  # This is used by meson.eclass.
	BUILD_LLVM_CONFIG="${spirv_llvm}/bin/llvm-config"
	append-cppflags "-I${spirv_llvm}/include"
	append-ldflags "-L${spirv_llvm}/lib"

	local emesonargs=(
		-Dllvm=enabled
		-Dcmake_prefix_path="${spirv_llvm}/lib/cmake"
		-Dshared-llvm=disabled
		-Dintel-clc=enabled
		-Dstatic-libclc=all

		-Dgallium-drivers=''
		-Dvulkan-drivers=''

		# Set platforms empty to avoid the default "auto" setting. If
		# platforms is empty meson.build will add surfaceless.
		-Dplatforms=''

		-Dglx=disabled
		-Dzstd=disabled

		--buildtype $(usex debug debug plain)
		-Db_ndebug=$(usex debug false true)
	)
	meson_src_configure
}

src_install() {
	newbin "${BUILD_DIR}"/src/intel/compiler/intel_clc intel_clc-"$(ver_cut 1-2)"
}
