# Copyright 2012 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI="7"

# Track all numbered kernel repos.
# This array is static due to tooling limitations. Specically, inheriting
# cros-kernel-versions doesn't consistently work in all of the ways that this
# ebuild is processed.
CROS_WORKON_COMMIT=("0a39174cfb27bc7c4dbaab3857e3c9835c43a440" "125bc86f71e7a2e913acccf326cb68800f10be6f" "83b089483a943aa67c1a6c5f7f732c8b517779af" "401525bdb1682ec39ad7500ed1de541d2e143b39" "8b992abc57c1e273247ff0c0dd62e44e8c3a90e1" "58bf52ffd7384e98a7ab6d6ba2a2557f5161e4d1" "e91c649615e82c3a8847e521cdf85728b4ab9b77")
CROS_WORKON_TREE=("224ea8f5855581f6fcb321e7a641db7fc96e22be" "0488374f14a43e6dd356c58db76a5f6681d3571d" "f423cc8a36888dcc87c4c6d6e2e18c129d371029" "0fb6b3796abbfe79d0bc3be4f1ba4d16932d485f" "d2729bc246646a4c3e2af5b9adbe85fedb05b4a7" "b43d8b71e5e2f818a4d2051079e55782665984a7" "13adbda40a8c6d8a8874ae0965cd5fc89a761a01")
CROS_WORKON_PROJECT=(
	"chromiumos/third_party/kernel"
	"chromiumos/third_party/kernel"
	"chromiumos/third_party/kernel"
	"chromiumos/third_party/kernel"
	"chromiumos/third_party/kernel"
	"chromiumos/third_party/kernel"
	"chromiumos/third_party/kernel"
)
CROS_WORKON_OPTIONAL_CHECKOUT=(
	"use kernel-5_4"
	"use kernel-5_10"
	"use kernel-5_15"
	"use kernel-6_1"
	"use kernel-6_6"
	"use kernel-6_10-enablement"
	"use kernel-6_12"
)
CROS_WORKON_LOCALNAME=(
	"kernel/v5.4"
	"kernel/v5.10"
	"kernel/v5.15"
	"kernel/v6.1"
	"kernel/v6.6"
	"kernel/v6.10-enablement"
	"kernel/v6.12"
)

inherit cros-workon cros-kernel-versions

DESCRIPTION="Chrome OS Kernel virtual package"
HOMEPAGE="http://src.chromium.org"

LICENSE="metapackage"
KEYWORDS="*"
S="${WORKDIR}"

# Check if the static arrays defined above need to be updated to reflect a
# change to cros-kernel-versions' CHROMEOS_KERNELS.
assert_localname_sync() {
	local k v actual expected expected_localname=()
	# shellcheck disable=SC2154
	for k in "${CHROMEOS_KERNELS[@]}"; do
		if [[ "${k}" =~ chromeos-kernel-([0-9]+_[0-9]+(-.*)?)$ ]]; then
			v="${BASH_REMATCH[1]//_/.}"
			expected="kernel/v${v}"
			expected_localname+=("${expected}")
			if [[ ! "${CROS_WORKON_LOCALNAME[*]}" =~ ${expected} ]]; then
				die "Append ${expected} to ${CATEGORY}/${PN} CROS_WORKON_LOCALNAME"
			fi
		fi
	done
	for actual in "${CROS_WORKON_LOCALNAME[@]}"; do
		if [[ ! "${expected_localname[*]}" =~ ${actual} ]]; then
			die "Remove ${actual} from ${CATEGORY}/${PN} CROS_WORKON_LOCALNAME"
		fi
	done
}

# shellcheck disable=SC2154
IUSE="${!CHROMEOS_KERNELS[*]}"
# exactly one of foo, bar, or baz must be set, but not several
REQUIRED_USE="^^ ( ${!CHROMEOS_KERNELS[*]} )"

# shellcheck disable=SC2154
RDEPEND="
	$(for v in "${!CHROMEOS_KERNELS[@]}"; do echo  "${v}? (  sys-kernel/${CHROMEOS_KERNELS[${v}]} )"; done)
"

# Add blockers so when migrating between USE flags, the old version gets
# unmerged automatically.
# shellcheck disable=SC2154
RDEPEND+="
	$(for v in "${!CHROMEOS_KERNELS[@]}"; do echo "!${v}? ( !sys-kernel/${CHROMEOS_KERNELS[${v}]} )"; done)
"

# Default to the latest kernel if none has been selected.
# TODO: This defaulting does not work. Fix or remove.
RDEPEND_DEFAULT="sys-kernel/chromeos-kernel-5_4"
# Here be dragons!
RDEPEND+="
	$(printf '!%s? ( ' "${!CHROMEOS_KERNELS[@]}")
	${RDEPEND_DEFAULT}
	$(printf '%0.s) ' "${!CHROMEOS_KERNELS[@]}")
"

src_unpack() {
	# Perform our assertions within a src_*() phase rather than at global
	# scope, so they only trigger when this package is in active use,
	# rather than when it is simply sourced.
	assert_localname_sync
}
