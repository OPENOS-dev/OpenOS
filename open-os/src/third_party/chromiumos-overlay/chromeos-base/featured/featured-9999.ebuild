# Copyright 2021 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7
CROS_WORKON_INCREMENTAL_BUILD="1"
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_LOCALNAME="platform2"
CROS_WORKON_OUTOFTREE_BUILD=1
# TODO(crbug.com/809389): Avoid directly including headers from other packages.
CROS_WORKON_SUBTREE="common-mk featured libhwsec-foundation .gn system_api"

PLATFORM_SUBDIR="featured"

PYTHON_COMPAT=( python3_11 )
inherit cros-workon platform cros-protobuf python-any-r1 tmpfiles user

DESCRIPTION="Chrome OS feature management service"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform2/+/HEAD/featured/"
LICENSE="BSD-Google"
KEYWORDS="~*"
IUSE="test"

COMMON_DEPEND="
	chromeos-base/libhwsec-foundation:=
	chromeos-base/session_manager-client:=
	dev-libs/openssl:=
"

RDEPEND="
	${COMMON_DEPEND}
	acct-group/feature-writers
	acct-user/feature-writers"

DEPEND="
	${COMMON_DEPEND}
	chromeos-base/system_api:=
	sys-apps/dbus:="

BDEPEND="
	chromeos-base/chromeos-dbus-bindings
	$(python_gen_any_dep '
		dev-python/jinja2[${PYTHON_USEDEP}]
	')
"

pkg_setup() {
	cros-workon_pkg_setup
	python-any-r1_pkg_setup
}

src_install() {
	platform_src_install

	into /
	dosbin "${OUT}"/featured

	insinto "/usr/$(get_libdir)/pkgconfig"
	dolib.so "${OUT}/lib/libfeatures.so"
	dolib.so "${OUT}/lib/libfeatures_c.so"
	local v="$(libchrome_ver)"
	./platform2_preinstall.sh "${OUT}" "${v}"
	doins "${OUT}/lib/libfeatures.pc"
	doins "${OUT}/lib/libfeatures_c.pc"

	insinto "/usr/include/featured"
	doins feature_export.h
	doins c_feature_library.h
	doins feature_library.h
	doins c_fake_feature_library.h
	doins fake_platform_features.h
	doins "${OUT}"/gen/featured/early_boot_state_checks.h

	# Install DBus configuration.
	insinto /etc/dbus-1/system.d
	doins share/org.chromium.featured.conf

	insinto /etc/init
	doins share/featured.conf share/featured-chrome-restart.conf

	dodir /etc/featured
	insinto /etc/featured
	fperms 0764 /etc/featured
	doins share/platform-features.json

	dotmpfiles tmpfiles.d/featured.conf

	local fuzzer_component_id="1096648"
	platform_fuzzer_install "${S}"/OWNERS \
			"${OUT}"/featured_json_feature_parser_fuzzer \
			--comp "${fuzzer_component_id}"
	# Test libraries.
	into /build
	dolib.so "${OUT}/lib/libfake_platform_features.so"
	dolib.so "${OUT}/lib/libc_fake_feature_library.so"
}

platform_pkg_test() {
	platform_test "run" "${OUT}/feature_library_test"
	platform_test "run" "${OUT}/service_test"
	platform_test "run" "${OUT}/store_impl_test"
}
