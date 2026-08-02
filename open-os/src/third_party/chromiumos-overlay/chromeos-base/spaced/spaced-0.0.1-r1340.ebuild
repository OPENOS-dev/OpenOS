# Copyright 2021 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_WORKON_COMMIT="8ea3af2134687a3c1a13767edbf649412bd26e3f"
CROS_WORKON_TREE=("518b50f8b6d01e95cbd933487ed7c6452ac4acb3" "f8c33d450bd37923ca39cdffe1f0d23740d27659" "0ba423cae8238e4b5b3d178d3ef4ebd9a001c96a" "f91b6afd5f2ae04ee9a2c19109a3a4a36f7659e6")
CROS_WORKON_INCREMENTAL_BUILD="1"
CROS_WORKON_PROJECT="chromiumos/platform2"
CROS_WORKON_LOCALNAME="platform2"
CROS_WORKON_OUTOFTREE_BUILD=1
CROS_WORKON_SUBTREE="common-mk spaced system_api .gn"

PLATFORM_SUBDIR="spaced"

inherit cros-workon platform cros-protobuf user

DESCRIPTION="Disk space information daemon for Chrome OS"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform2/+/HEAD/spaced/"

LICENSE="BSD-Google"
KEYWORDS="*"
IUSE="+seccomp"

RDEPEND="
	sys-apps/rootdev:=
"
DEPEND="
	${RDEPEND}
	chromeos-base/system_api:=
"

BDEPEND="
	chromeos-base/chromeos-dbus-bindings
	chromeos-base/minijail
"

pkg_preinst() {
	enewuser "spaced"
	enewgroup "spaced"
}

src_install() {
	platform_src_install
	platform_install_dbus_client_lib

	if use seccomp; then
		local policy="seccomp/spaced-seccomp-${ARCH}.policy"
		local policy_out="${ED}/usr/share/policy/spaced-seccomp.policy.bpf"
		dodir /usr/share/policy
		compile_seccomp_policy \
			--arch-json "${SYSROOT}/build/share/constants.json" \
			--default-action trap "${policy}" "${policy_out}" \
			|| die "failed to compile seccomp policy ${policy}"
	fi
}
