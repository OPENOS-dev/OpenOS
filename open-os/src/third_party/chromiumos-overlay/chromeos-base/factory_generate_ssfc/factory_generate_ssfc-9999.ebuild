# Copyright 2023 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_WORKON_INCREMENTAL_BUILD=1
CROS_WORKON_LOCALNAME="platform2"
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_OUTOFTREE_BUILD=1
# TODO(crbug.com/809389): Avoid directly including headers from other packages.
CROS_WORKON_SUBTREE="common-mk chromeos-config hardware_verifier libec libmems mojo_service_manager rmad .gn"
PLATFORM_SUBDIR="rmad/tools/factory_generate_ssfc"

inherit cros-workon cros-unibuild platform user cros-protobuf

DESCRIPTION="SSFC probe tool for factory environment."
HOMEPAGE=""

LICENSE="BSD-Google"
KEYWORDS="~*"

COMMON_DEPEND="
	chromeos-base/cryptohome-client:=
"

RDEPEND="
	${COMMON_DEPEND}
	chromeos-base/runtime_probe:=
"

DEPEND="
	${COMMON_DEPEND}
	chromeos-base/chromeos-config-tools:=
	chromeos-base/hardware_verifier:=
	chromeos-base/hardware_verifier_proto:=
	chromeos-base/iioservice:=
	chromeos-base/libec:=
	chromeos-base/libiioservice_ipc:=
	chromeos-base/libmems:=[test?]
	chromeos-base/mojo_service_manager:=
	chromeos-base/runtime_probe-client:=
	chromeos-base/shill-client:=
	chromeos-base/system_api:=
	chromeos-base/tpm_manager-client:=
	dev-libs/re2:=
"
