# Copyright 2023 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_WORKON_COMMIT="1b7a3e54d8c660cddc38d7869af656d801d22b19"
CROS_WORKON_TREE=("518b50f8b6d01e95cbd933487ed7c6452ac4acb3" "16839d1d0fcecb39e52d1bc3a65aa02d62c1ee6d" "06ecc9190ad3c2a88e33009e8b2d7d30fc099c2e" "4934b6b332f2a3db7a26bad9f888607a4f12b440" "47c9d8a1ba175459aaf9f255c44f91df349864ab" "43eb4f30218ee6fc055f185786d914bccd668086" "088335464ec4a31ef632c82aae4b5e5483e2e19d" "f91b6afd5f2ae04ee9a2c19109a3a4a36f7659e6")
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
KEYWORDS="*"

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
