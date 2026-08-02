# Copyright 2020 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

# @ECLASS: cros-racc.eclass
# @BLURB: helper eclass for building Chromium packages of RACC
# @DESCRIPTION:
# Packages src/platform2/{hardware_verifier,runtime_probe} are in active
# development.  We have to add board-specific rules manually.

EXPORT_FUNCTIONS src_compile src_install

inherit cros-protobuf

DEPEND="
	chromeos-base/system_api:=
	chromeos-base/hardware_verifier_proto:=
"

# @FUNCTION: cros-racc_src_compile
# @DESCRIPTION:
# Remove all indents, line breaks and spaces in json file to reduce disk usage.
cros-racc_src_compile() {
	einfo "cros-racc src_compile"

	local CMD_MINIFY_JSON=("jq" "-c" ".")
	local BUILD_ROOT="${WORKDIR}/build"
	local config
	if [[ -d "${FILESDIR}/runtime_probe/" ]]; then
		while read -r -d $'\0' config; do
			mkdir -p "$(dirname "${BUILD_ROOT}/${config}")"
			"${CMD_MINIFY_JSON[@]}" \
				< "${FILESDIR}/${config}" > "${BUILD_ROOT}/${config}" ||
				die "Failed to minify json file: ${config}"
		done < <(find -H "${FILESDIR}" -path "*/runtime_probe/*.json" -type f -printf "%P\0" || die)
	fi
}

# @FUNCTION: cros-racc_src_install
# @DESCRIPTION:
# Install AVL runtime verification payload files.
# See go/cros-probe for more details.
cros-racc_src_install() {
	einfo "cros-racc src_install"

	if [[ -d "${WORKDIR}/build/runtime_probe/" ]]; then
		insinto /etc/runtime_probe
		doins -r "${WORKDIR}/build/runtime_probe/"*
	fi

	if [[ -d "${FILESDIR}/runtime_probe/" ]]; then
		local config
		while read -r -d $'\0' config; do
			insinto "$(dirname "/etc/${config}")"
			doins "${FILESDIR}/${config}"
		done < <(find -H "${FILESDIR}" -path "*/runtime_probe/*.json" -type l -printf "%P\0" || die)
		while read -r -d $'\0' config; do
			insinto "$(dirname "/etc/${config}")"
			doins "${FILESDIR}/${config}"
		done < <(find -H "${FILESDIR}" -path "*/runtime_probe/*.json.enc" -type f -printf "%P\0" || die)
	fi

	if [[ -e "${FILESDIR}/hw_verification_spec.prototxt" ]]; then
		insinto /etc/hardware_verifier
		doins "${FILESDIR}/hw_verification_spec.prototxt"
	fi

	local hardware_verifier_proto_dir="${SYSROOT}/usr/include/chromeos/hardware_verifier"
	local hardware_verifier_proto="${hardware_verifier_proto_dir}/hardware_verifier.proto"
	local runtime_probe_proto_dir="${SYSROOT}/usr/include/chromeos/dbus/runtime_probe"
	local runtime_probe_proto="${runtime_probe_proto_dir}/runtime_probe.proto"

	if [[ ! -e "${hardware_verifier_proto}" || ! -e "${runtime_probe_proto}" ]]; then
		die "${hardware_verifier_proto} or ${runtime_probe_proto} does not exist."
	fi

	local encoding_spec
	while read -r -d $'\0' encoding_spec; do
		local output_dir="${WORKDIR}/build/encoding_spec/$(dirname "${encoding_spec}")"
		mkdir -p "${output_dir}" || die

		protoc --encode=hardware_verifier.EncodingSpec \
			--proto_path="${hardware_verifier_proto_dir}" \
			--proto_path="${runtime_probe_proto_dir}" \
			"${hardware_verifier_proto}" \
			< "${FILESDIR}/${encoding_spec}" > "${output_dir}/encoding_spec.pb" ||
			die "Failed to encode txtpb file: ${encoding_spec}"
		insinto "$(dirname "/etc/${encoding_spec}")"
		doins "${output_dir}/encoding_spec.pb"
	done < <(find -H "${FILESDIR}" -path "*/runtime_probe/*/encoding_spec.txtpb" -type f -printf "%P\0" || die)
}
