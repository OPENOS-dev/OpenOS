# Copyright 2024 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI="7"

CROS_WORKON_COMMIT=("e67643c64a105f6f744b007eb857f381ace07e8e" "aa6e5ff2053d88a326eebc1e1a1bcdc217633c5a" "9537e373c71c26c5495be60d267dff5eb88b180f")
CROS_WORKON_TREE=("518b50f8b6d01e95cbd933487ed7c6452ac4acb3" "f91b6afd5f2ae04ee9a2c19109a3a4a36f7659e6" "26973af5cd61a498d842fa9a8956b519afcc0478" "6fadd8addab8504349cdeefe51b583b97c2ae7f4")
CROS_WORKON_PROJECT=(
	"chromiumos/platform2"
	"platform/system/tools/sysprop"
	"aosp/platform/system/libbase"
)
CROS_WORKON_LOCALNAME=(
	"../platform2"
	"../aosp/system/tools/sysprop"
	"../aosp/system/libbase"
)
CROS_WORKON_DESTDIR=(
	"${S}/platform2"
	"${S}/platform2/external/sysprop"
	"${S}/platform2/external/sysprop/libbase"
)
CROS_WORKON_SUBTREE=("common-mk .gn" "" "")
PLATFORM_SUBDIR="external/sysprop"

WANT_LIBCHROME=no
WANT_LIBBRILLO=no

inherit cros-workon cros-toolchain-funcs platform

DESCRIPTION='sysprop_cpp is a build time tool to generate libraries from sysprop
descrption files.'
HOMEPAGE='https://android.googlesource.com/platform/system/tools/sysprop/+/refs/heads/main'

LICENSE="Apache-2.0"
KEYWORDS="*"

DEPEND="
	dev-libs/libfmt:=
	dev-libs/protobuf:=
"
RDEPEND="${DEPEND}"

PATCHES=(
	"${FILESDIR}/0001-CHROMIUM-floss-Add-BUILD.gn-file.patch"
	"${FILESDIR}/0002-CHROMIUM-floss-Update-logging-with-only-functions-ne.patch"
	"${FILESDIR}/0003-UPSTREAM-Add-support-for-default-values-in-sysprop-definition.patch"
)

src_configure() {
	append-cxxflags "-std=c++17"
	# Suppress warning about usage of sysprop::Scope::System
	append-cxxflags "-Wno-deprecated-declarations"
	append-ldflags "-lprotobuf -lfmt -lstdc++"

	platform_src_configure "--target_os=chromeos"
}


src_compile() {
	platform_src_compile
}

src_install() {
	dobin "$(cros-workon_get_build_dir)/out/Default/sysprop_cpp"
}
