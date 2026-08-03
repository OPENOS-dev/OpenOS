# Copyright 2014 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI="7"

CROS_WORKON_PROJECT=("chromiumos/platform2" "chromiumos/platform/libchrome")
CROS_WORKON_LOCALNAME=("platform2" "platform/libchrome")
CROS_WORKON_EGIT_BRANCH=("main" "main")
CROS_WORKON_DESTDIR=("${S}/platform2" "${S}/platform2/libchrome")
CROS_WORKON_SUBTREE=("common-mk .gn" "")

CROS_PROTOBUF_APPLY_DEFAULT_DEPS=0
WANT_LIBCHROME="no"
inherit cros-workon platform cros-protobuf flag-o-matic

DESCRIPTION="Chrome base/ and dbus/ libraries extracted for use on Chrome OS"
HOMEPAGE="http://dev.chromium.org/chromium-os/packages/libchrome"
SRC_URI=""

LICENSE="BSD-Google"
KEYWORDS="~*"
IUSE="cros_host +crypto +dbus fuzzer ipcz +mojo"

PLATFORM_SUBDIR="libchrome"

# libchrome dynamically links against libperfetto.so (via pkg_deps), so the
# shared library is required at runtime.
RDEPEND="
	>=chromeos-base/perfetto-21.0-r4:=
	>=dev-cpp/abseil-cpp-20240116.3-r3:=
	dev-libs/double-conversion:=
	dev-libs/glib:2=
	dev-libs/libevent:=
	dev-libs/modp_b64:=
	crypto? (
		dev-libs/nspr:=
		dev-libs/nss:=
		dev-libs/openssl:=
	)
	dbus? (
		sys-apps/dbus:=
		${CROS_PROTOBUF_DEPS}
	)
	dev-libs/re2:=
"
# RDEPEND is inherited here. perfetto is required in DEPEND to provide headers
# and symlinks for linking against libperfetto.so during compilation.
DEPEND="${RDEPEND}
	dev-cpp/gtest:=
"

# perfetto provides the /usr/libexec/protozero_plugin executable which is
# executed by protoc on the build host to generate code for
# chrome_track_event.proto.
#
# b/471065324: abseil, glib and libevent are in `pkg_deps` of libbase-core and are needed in `gn gen`.
BDEPEND="
	chromeos-base/perfetto
	${CROS_PROTOC_DEPS}
	dev-cpp/abseil-cpp:=
	dev-libs/glib:2=
	dev-libs/libevent:=
"

# libmojo depends on libbase-crypto.
REQUIRED_USE="mojo? ( crypto )"

src_prepare() {
	# Remove patches that do not apply.
	while read -ra patch_config; do
		local patch="${patch_config[0]}"
		local use_flag="${patch_config[1]}"
		if [ -z "${use_flag}" ]; then
			die "Missing use flag for patch: ${patch}"
		fi
		if ! use "${use_flag}"; then
			einfo "Skip ${patch}"
			rm "${S}/libchrome_tools/patches/${patch}" || die "failed to remove patch ${patch}"
		fi
	done < <(grep -E '^[^#]' "${S}/libchrome_tools/patches/patches.config")

	"${S}/libchrome_tools/apply_patches.py" "--ebuild" || die
	"${S}/libchrome_tools/disable_warnings.py" || die

	eapply_user
}

src_configure() {
	cros_optimize_package_for_speed

	platform_src_configure
}

src_test() {
	pushd libchrome_tools || die
	python3 -m unittest check_libchrome_test || die "failed python3 check-libchrome-test.py"
	python3 -m unittest disable_warnings_test || die "failed python3 disable_warnings_test"
	pushd uprev || die
	python3 ./run_tests.py || die "failed python3 libchrome/uprev/run_tests.py"
	popd || die
	pushd developer-tools || die
	python3 -m unittest test_change_header || die "failed python3 test_change_headerpy"
	popd || die
	popd || die
}

src_install() {
	platform_src_install

	dolib.so "${OUT}"/lib/libbase*.so
	dolib.a "${OUT}"/libbase*.a

	insinto "/usr/$(get_libdir)/pkgconfig"
	doins "${OUT}"/obj/libchrome/libchrome*.pc

	# Install libmojo.
	if use mojo; then
		# Install binary.
		dolib.so "${OUT}"/lib/libmojo.so

		# Install libmojo.pc.
		insinto "/usr/$(get_libdir)/pkgconfig"
		doins "${OUT}"/obj/libchrome/libmojo.pc

		# Install generate_mojom_bindings.
		# TODO(hidehiko): Clean up tools' install directory.
		insinto /usr/src/libmojo/mojo
		doins -r mojo/public/tools/bindings/*
		doins -r mojo/public/tools/mojom/*
		doins build/action_helpers.py
		doins build/gn_helpers.py
		doins build/zip_helpers.py
		doins -r build/android/gyp/util
		doins -r build/android/pylib
		exeinto /usr/src/libmojo/mojo
		doexe libchrome_tools/mojom_generate_type_mappings.py

		insinto /usr/src/libmojo/third_party
		doins -r third_party/jinja2
		doins -r third_party/markupsafe
		doins -r third_party/ply

		# Mark scripts executable.
		fperms +x \
			/usr/src/libmojo/mojo/generate_type_mappings.py \
			/usr/src/libmojo/mojo/mojom_bindings_generator.py \
			/usr/src/libmojo/mojo/mojom_parser.py
	fi
}
