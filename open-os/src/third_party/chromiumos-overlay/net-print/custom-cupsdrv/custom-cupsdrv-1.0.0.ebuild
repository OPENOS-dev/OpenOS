# Copyright 2024 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

inherit eutils cros-sanitizers multilib

GIT_SHA="67c52df480341f8606415653c38bce38bd028940"
DESCRIPTION="CUPS filter and PPD files for Custom printers"
HOMEPAGE="http://www.custom4u.it/"
SRC_URI="https://github.com/CustomOpenSource/CUPS_drivers/archive/${GIT_SHA}.tar.gz -> custom-cupsdrv-${PV}-${GIT_SHA}.tar.gz"

LICENSE="Apache-2.0"
SLOT="0"
KEYWORDS="*"
IUSE=""

DEPEND="net-print/cups-filters"
RDEPEND="${DEPEND}"

S="${WORKDIR}/CUPS_drivers-${GIT_SHA}"

compile_makefile() {
	local dir=$1
	cd "${dir}" || die "Failed to navigate to ${dir}"
	sed -i "s/gcc/\$(CC) \$(CFLAGS) \$(CPPFLAGS)/g" makefile || die "sed failed"
	emake || die "Failed to compile makefile in ${dir}"
	cd - || die "Failed to return to the original directory"
}

src_configure() {
	sanitizers-setup-env
	append-lfs-flags
	default
}

src_compile() {
	# Copy the output of PtPLm library compilation to use it when compiling VKP80III
	compile_makefile "${S}/PtPLm"
	cp -r "${S}/PtPLm/out" "${S}/VKP80III/lib"

	# Iterate over each subdirectory containing the makefiles
	for dir in "${S}"/*/; do
		if [ -d "${dir}" ] && [ "$(basename "${dir}")" != "PtPLm" ]; then
			compile_makefile "${dir}"
		fi
	done
}

src_install() {
	exeinto "/usr/libexec/cups/filter"
	binary_names=("rasterto" "rasterTo")

	for dir in "${S}"/*/; do
		if [ -d "${dir}" ]; then
			dir_basename=$(basename "${dir}")
			for name in "${binary_names[@]}"; do
				executable_path="${dir}bin/${name}${dir_basename}"
				if [[ -x "${executable_path}" ]]; then
					doexe "${executable_path}"
				fi
			done
		fi
	done

	insinto "/usr/$(get_libdir)"
	doins "${S}/VKP80III/lib/libPtPLm.so"
}
