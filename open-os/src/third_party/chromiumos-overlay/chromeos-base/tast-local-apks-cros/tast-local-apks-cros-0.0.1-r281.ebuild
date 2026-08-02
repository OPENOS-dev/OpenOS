# Copyright 2020 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI="7"

CROS_WORKON_COMMIT="f64befac13a4f5e457d0dcefc6674318ec719a86"
CROS_WORKON_TREE="f504063396a99a9133431049802d989ca59756e0"
CROS_WORKON_PROJECT="chromiumos/platform/tast-tests"
CROS_WORKON_LOCALNAME="platform/tast-tests"
CROS_WORKON_SUBTREE="android"

inherit cros-workon

DESCRIPTION="Compiled apks used by local Tast tests in the cros bundle"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/platform/tast-tests/+/HEAD/android"

LICENSE="BSD-Google GPL-3"
SLOT="0"
KEYWORDS="*"

BDEPEND="
	app-arch/zip
	chromeos-base/android-sdk
	dev-util/gn
	dev-util/ninja
	virtual/jdk:11
"
RDEPEND=""
DEPEND="${RDEPEND}"
OUT=$(cros-workon_get_build_dir)

src_compile() {
	# Make sure we don't use JDK 8.
	export GENTOO_VM=openjdk-bin-11

	gn gen "${OUT}" --root="${S}"/android || die "gn failed"
	ninja -C "${OUT}" || die "build failed"
}

src_install() {
	if [ ! -d "${OUT}/apks" ]; then
		ewarn "There is no apk."
		ewarn "If you want to add a helper APK, add it under tast-tests/android"
		ewarn "and modify BUILD.gn."
		return
	fi
	insinto /usr/libexec/tast/apks/local/cros
	doins "${OUT}"/apks/*
}
