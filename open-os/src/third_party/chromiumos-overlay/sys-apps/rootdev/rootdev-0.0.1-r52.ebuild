# Copyright 2010 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7
CROS_WORKON_COMMIT="9c3929cc9f7b9d3fd11783bff6f90ebd52847aaa"
CROS_WORKON_TREE="60b082d94ded6e7f6c9b76b75150b9f3431bb27a"
CROS_WORKON_PROJECT="chromiumos/third_party/rootdev"
CROS_WORKON_OUTOFTREE_BUILD="1"

inherit cros-toolchain-funcs cros-sanitizers cros-workon cros-constants

DESCRIPTION="Chrome OS root block device tool/library"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/third_party/rootdev/"
SRC_URI=""

LICENSE="BSD-Google"
KEYWORDS="*"
IUSE="-asan"

src_configure() {
	sanitizers-setup-env
	tc-export CC
	default
}

src_compile() {
	emake OUT="${WORKDIR}"
}

platform2_test() {
	local cmd=(
		"${CHROOT_SOURCE_ROOT}/src/platform2/common-mk/platform2_test.py"
		--run_as_root
		--sysroot "${SYSROOT}"
		"$@"
	)
	echo "+ ${cmd[*]}"
	"${cmd[@]}" || die
}

src_test() {
	cd "${T}" || die
	platform2_test --action=pre_test
	platform2_test --action=run "${S}/rootdev_test.sh" "${WORKDIR}/rootdev"
}

src_install() {
	cd "${WORKDIR}"
	dobin rootdev
	dolib.so librootdev.so*
	insinto /usr/include/rootdev
	doins "${S}"/rootdev.h
}
