# Copyright 2024 The ChromiumOS Authors. All rights reserved.
# Distributed under the terms of the GNU General Public License v3

EAPI=7

PYTHON_COMPAT=( python3_11 )

CROS_WORKON_PROJECT=(
	"chromiumos/platform/dev-util"
	"chromiumos/config"
)
CROS_WORKON_LOCALNAME=(
	"../platform/dev"
	"../config"
)
CROS_WORKON_DESTDIR=(
	"${S}/platform/dev"
	"${S}/config"
)

inherit cros-workon cros-constants python-any-r1

DESCRIPTION="Centralized Suite proto builder"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/config/+/HEAD/test/suite_sets/"

LICENSE="BSD-Google"
KEYWORDS="~*"
IUSE="internal"

BDEPEND="
	$(python_gen_any_dep '
		dev-python/protobuf-python[${PYTHON_USEDEP}]
		chromeos-base/cros-config-api[${PYTHON_USEDEP}]
	')
"

DEPEND="
	chromeos-base/autotest-all
	chromeos-base/tast-local-tests
	chromeos-base/tast-remote-tests
	internal? ( dev-util/centralized-suites-private )
"

TEST_METADATA_FILES=(
	"usr/local/build/autotest/autotest_metadata.pb autotest.pb"
	"usr/share/tast/metadata/local/cros.pb tast_local.pb"
	"build/share/tast/metadata/local/crosint.pb tast_local_private.pb"
	"usr/share/tast/metadata/remote/cros.pb tast_remote.pb"
	"usr/local/build/gtest/gtest_metadata.pb gtest.pb"
)

python_check_deps() {
	python_has_version -b \
		"dev-python/protobuf-python[${PYTHON_USEDEP}]" \
		"chromeos-base/cros-config-api[${PYTHON_USEDEP}]"
}

src_compile() {
	local test_metadata_dir="${S}/test_metadata"
	mkdir "${test_metadata_dir}"
	for test_metadata_info in "${TEST_METADATA_FILES[@]}"; do
		relative_path=$(echo "${test_metadata_info}" | cut -d ' ' -f 1)
		dest_file=$(echo "${test_metadata_info}" | cut -d ' ' -f 2)
		abs_path="${SYSROOT}/${relative_path}"
		if [ -f "${abs_path}" ]; then
			cp "${abs_path}" "${test_metadata_dir}/${dest_file}" || die
		else
			ewarn "Skipping metadata file: ${abs_path} because it does not exist"
		fi
	done

	local script_dir="${S}/platform/dev/src/chromiumos/test/python/src/tools"
	local csuite_proto_dir="${S}/config/test/suite_sets/generated"
	local csuite_private_proto_dir="${SYSROOT}/usr/share/centralized-suites-private"
	"${EPYTHON}" "${script_dir}/prep_centralized_suites.py" \
		--suite_sets_input \
			"${csuite_proto_dir}/suite_sets.jsonpb" \
			"${csuite_private_proto_dir}/suite_sets.jsonpb" \
		--suite_sets_output "${S}/suite_sets.pb" \
		--suites_input \
			"${csuite_proto_dir}/suites.jsonpb" \
			"${csuite_private_proto_dir}/suites.jsonpb" \
		--suites_output "${S}/suites.pb" \
		--test_metadata_dir "${test_metadata_dir}" || die
}

src_install() {
	insinto "/usr/share/centralized-suites/"
	doins "${S}/suites.pb"
	doins "${S}/suite_sets.pb"
}
