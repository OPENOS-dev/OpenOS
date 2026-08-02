# Copyright 2022 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_WORKON_COMMIT="362404e596160add78f63bc42ff2081b91941af5"
CROS_WORKON_TREE=("518b50f8b6d01e95cbd933487ed7c6452ac4acb3" "f3ced63dac603322782ea1cfe567ea397a0d353c" "3d01071ecc84cb5a5781cd4a17dc0efe279a6169" "f91b6afd5f2ae04ee9a2c19109a3a4a36f7659e6")
CROS_WORKON_INCREMENTAL_BUILD=1
CROS_WORKON_LOCALNAME="platform2"
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_OUTOFTREE_BUILD=1
CROS_WORKON_SUBTREE="common-mk diagnostics/cros_minidiag metrics .gn"

PLATFORM_SUBDIR="diagnostics/cros_minidiag"

inherit cros-fuzzer cros-sanitizers cros-workon platform

DESCRIPTION="User space utilities related to MiniDiag"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform2/+/HEAD/diagnostics/cros_minidiag"

LICENSE="BSD-Google"
KEYWORDS="*"
IUSE="fuzzer"

DEPEND="
	>=chromeos-base/metrics-0.0.1-r3152:=
	dev-libs/re2:=
"

RDEPEND="
	${DEPEND}
	sys-apps/coreboot-utils:=
	!<chromeos-base/chromeos-init-0.0.26
"

src_configure() {
	sanitizers-setup-env
	platform_src_configure
}

src_install() {
	platform_src_install

	# Install fuzzers.
	local fuzzer_component_id="982097"
	platform_fuzzer_install "${S}"/OWNERS "${OUT}"/minidiag_utils_fuzzer \
		--comp "${fuzzer_component_id}"
}
