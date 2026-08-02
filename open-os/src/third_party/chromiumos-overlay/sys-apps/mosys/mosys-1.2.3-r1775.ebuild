# Copyright 2012 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

CROS_WORKON_COMMIT=("e67643c64a105f6f744b007eb857f381ace07e8e" "1b96006e0b49c21198f45316bd8da5e3027c60f1")
CROS_WORKON_TREE=("518b50f8b6d01e95cbd933487ed7c6452ac4acb3" "ff26b4fa553ad842c7c75ee26519324252f0aa5c")
CROS_WORKON_PROJECT=(
	"chromiumos/platform2"
	"chromiumos/platform/mosys"
)
CROS_WORKON_LOCALNAME=(
	"../platform2"
	"../platform/mosys"
)
CROS_WORKON_DESTDIR=(
	"${S}/platform2"
	"${S}/platform/mosys"
)
CROS_WORKON_INCREMENTAL_BUILD=1
CROS_WORKON_SUBTREE=(
	"common-mk"
	""
)

WANT_LIBCHROME="no"
WANT_LIBBRILLO="no"

inherit meson flag-o-matic cros-toolchain-funcs cros-workon platform

PLATFORM_NAMES=(
	"Arm"
	"Asurada"
	"Cherry"
	"Corsola"
	"Generic"
	"Geralt"
	"Kukui"
	"Oak"
	"Trogdor"
)
PLATFORM_NAME_USE_FLAGS=()
for platform_name in "${PLATFORM_NAMES[@]}"; do
	PLATFORM_NAME_USE_FLAGS+=("mosys_platform_${platform_name,,}")
done

DESCRIPTION="Utility for obtaining various bits of low-level system info"
HOMEPAGE="http://mosys.googlecode.com/"

LICENSE="BSD-Google BSD Apache-2.0 MIT ISC Unlicense"
SLOT="0/0"
KEYWORDS="*"
IUSE="
	unibuild
	${PLATFORM_NAME_USE_FLAGS[*]}
"
REQUIRED_USE="
	unibuild
	^^ ( ${PLATFORM_NAME_USE_FLAGS[*]} )
"

RDEPEND="
	chromeos-base/ec-utils
	dev-util/cmocka
"
DEPEND="
	dev-util/cmocka
"

src_unpack() {
	cros-workon_src_unpack
	export PLATFORM_TOOLDIR="${S}/platform2/common-mk"
	S+="/platform/mosys"
}

src_configure() {
	local platform_intf=""
	local emesonargs=()

	for ((i = 0; i < ${#PLATFORM_NAMES[@]}; i++)); do
		if use "${PLATFORM_NAME_USE_FLAGS[${i}]}"; then
			platform_intf="${PLATFORM_NAMES[${i}]}"
			break
		fi
	done

	if [[ -n "${platform_intf}" ]]; then
		emesonargs+=(
			"-Dplatform_intf=${platform_intf}"
		)
	fi

	# Necessary to enable LTO.  See crbug.com/1082378.
	append-ldflags "-O2"

	meson_src_configure
}

src_compile() {
	meson_src_compile
}

platform_pkg_test() {
	local tests=(
		cbi_unittest
		file_unittest
	)
	local test_bin
	for test_bin in "${tests[@]}"; do
		platform_test "run" \
			"${BUILD_DIR}/unittests/${test_bin}"
	done
}

src_install() {
	dosbin "${BUILD_DIR}/mains/mosys"
}
