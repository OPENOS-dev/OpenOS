# Copyright 2020 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

DESCRIPTION="Android SDK"
HOMEPAGE="https://developer.android.com"

# NOTE: Due to possible licensing issues, only use AOSP SDK:
# https://ci.android.com/builds/branches/aosp-sdk-release/grid?
SRC_URI="https://ci.android.com/builds/submitted/${PV}/sdk/latest/sdk-repo-linux-platforms-${PV}.zip
	https://ci.android.com/builds/submitted/${PV}/sdk/latest/sdk-repo-linux-build-tools-${PV}.zip"

LICENSE="AOSP-SDK Apache-2.0 CPL-1.0 EPL-1.0 FTL GPL-2+ GPL-2-with-classpath-exception LGPL-2+ LGPL-2.1+ MPL-1.1 NPL-1.1"
SLOT="0"
KEYWORDS="-* amd64"
IUSE=""

DEPEND=""
# CTS P needs Java 8 or 9 to run the tests. CTS R needs Java 9 or later.
# Include both JDK8 and JDK11 in the chroot to make sure both CTS can run in chroot.
RDEPEND="
	<=virtual/jdk-9
	>=virtual/jdk-9"
BDEPEND="app-arch/unzip"

ANDROID_SDK_DIR="/opt/android-sdk"

S="${WORKDIR}"

src_unpack() {
	mkdir build-tools platforms || die
	cd "${WORKDIR}/build-tools" || die
	unpack "sdk-repo-linux-build-tools-${PV}.zip"
	cd "${WORKDIR}/platforms" || die
	unpack "sdk-repo-linux-platforms-${PV}.zip"
}

src_install() {
	# NOTE: The two downloaded zips use "android-VanillaIceCream" for their directories.
	# It seems that they take the name of the latest Android SDK at the
	# moment it was built, even if they were compiled from a different
	# branch. See build.prop: notice conflict between SDK version and name:
	# https://ci.android.com/builds/submitted/10764648/sdk/latest/view/build.prop

	# Zips to be installed:
	#  - Android SDK 33: both build-tools and platforms

	# License file for platforms and build-tools is in licenses/AOSP-SDK
	insinto "${ANDROID_SDK_DIR}"

	doins -r platforms
	exeinto "${ANDROID_SDK_DIR}/build-tools"
	doexe build-tools/android-VanillaIceCream/aapt2
	doexe build-tools/android-VanillaIceCream/apksigner
	doexe build-tools/android-VanillaIceCream/d8
	insinto "${ANDROID_SDK_DIR}/build-tools"
	doins -r build-tools/android-VanillaIceCream/lib
}
