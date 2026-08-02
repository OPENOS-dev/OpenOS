# Copyright 2020 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

PYTHON_COMPAT=( python3_{8..11} )
inherit cros-factory python-any-r1

# @ECLASS: cros-factory-board.eclass
# @MAINTAINER:
# The ChromiumOS Authors <chromium-os-dev@chromium.org>
# @BUGREPORTS:
# Please report bugs via http://crbug.com/new (with label Build)
# @VCSURL: https://chromium.googlesource.com/chromiumos/overlays/chromiumos-overlay/+/HEAD/eclass/@ECLASS@
# @BLURB: Eclass to help creating per-project factory resources.

# Check for EAPI 7+
case "${EAPI:-0}" in
	0|1|2|3|4|5|6)
		die "unsupported EAPI (${EAPI}) in eclass (${ECLASS})" ;;
esac

# @ECLASS-VARIABLE: PROGRAM
# @PRE_INHERIT
# @REQUIRED
# @DESCRIPTION:
# Name of the program under src/program
: "${PROGRAM:=alpha}"

# @ECLASS-VARIABLE: PROJECTS
# @PRE_INHERIT
# @REQUIRED
# @DESCRIPTION:
# Names of the projects under src/project/$PROGRAM/ that will be
# included in this build.
: "${PROJECTS:=(one two three)}"

IUSE="racc"

DEPEND="
	!racc? (
		chromeos-base/factory_runtime_probe
	)
"

# @FUNCTION: cros-factory-board_build_config
# @USAGE:cros-factory-board_build_config
# @DESCRIPTION:
# Build factory configuration file model_sku.json
cros-factory-board_build_config() {
	local project
	local factory_gen_dir

	for project in "${PROJECTS[@]}"; do
		if [[ ! -d "${S}/${project}" ]]; then
			continue
		fi
		einfo "Generating model_sku.json for project:${project}"
		factory_gen_dir="${S}/${project}/factory/generated"
		mkdir -p "${factory_gen_dir}"
		"${EPYTHON}" "${S}/config/payload_utils/factory_config_proto_converter.py" \
			--output "${factory_gen_dir}/model_sku.json" \
			--program_config "${S}/program/${PROGRAM}/generated/config.jsonproto" \
			--project_configs "${S}/${project}/generated/config.jsonproto" \
			|| die "Failed to build factory config"
	done
}

# @FUNCTION: cros-factory-board_install_project_config
# @USAGE:
# @DESCRIPTION:
# Install project config into the toolkit and the factory bundle.
# @EXAMPLE:
# To install project config:
#
# @CODE
#  cros-factory-board_install_project_config
# @CODE
cros-factory-board_install_project_config() {
	einfo "Installing project_config ..."
	local configs=()
	local project_config_workdir="${WORKDIR}/project_config"
	local package_dir="${WORKDIR}/project_config_result"
	local package="${package_dir}/project_config.tar.gz"
	mkdir -p "${project_config_workdir}" "${package_dir}"
	local config_source_dir
	while IFS= read -r -d '' config_source_dir; do
		local project_dir=$(dirname -- "$(dirname -- "${config_source_dir}")")
		local overlay_source_dir="${project_dir}/factory/files"
		local PROJECT=$(basename -- "${project_dir}")
		local file
		for file in "${config_source_dir}"/*; do
			mkdir -p "${overlay_source_dir}/project_config" || die
			local new_file="${PROGRAM}_${PROJECT}_$(basename -- "${file}")"
			ln -f "${file}" "${project_config_workdir}/${new_file}" || die
			ln -f "${file}" "${overlay_source_dir}/project_config/${new_file}" || die
			configs+=("${new_file}")
		done
	done < <(find "${S}" -type d -path "*factory/generated" -print0)
	# tar will fail to create empty archive so we add a file here.
	if [[ ${#configs[@]} -eq 0 ]]; then
		local new_file="this_is_an_empty_archive"
		touch "${project_config_workdir}/${new_file}" || die
		configs+=("${new_file}")
	fi
	# Create a project config tar as a bundle component that can be uploaded to
	# Dome or put inside RMA shim.
	tar -I pigz -cf "${package}" \
		--owner=0 --group=0 -C "${project_config_workdir}" "${configs[@]}" || die
	insinto "/usr/local/factory/bundle/project_config"
	doins "${package}"
	# Install project_config into the toolkit directly.
	# TODO(cyueh) Remove unibuild-project-config after we enabling downloading
	# project specific toolkit from CPFE or easy bundle creation.
	factory_create_resource "unibuild-project-config" "${WORKDIR}" "." \
		"project_config"
}

# @FUNCTION: cros-factory-board_install_project_overlay
# @USAGE:
# @DESCRIPTION:
# Install project overlay into the toolkit.
# @EXAMPLE:
# To install project overlay:
#
# @CODE
#  cros-factory-board_install_project_overlay
# @CODE
cros-factory-board_install_project_overlay() {
	einfo "Installing project overlay ..."
	local overlay_source_dir

	while IFS= read -r -d '' overlay_source_dir; do
		local project_dir=$(dirname -- "$(dirname -- "${overlay_source_dir}")")
		local PROJECT=$(basename -- "${project_dir}")
		# Install project overlay into the toolkit directly.
		factory_create_resource "project-${PROGRAM}-${PROJECT}-overlay" \
			"${overlay_source_dir}" "." "."
	done < <(find "${S}" -type d -path "*factory/files" -print0)
}

# @FUNCTION: cros-factory-board_install_factory_runtime_probe
# @USAGE:
# @DESCRIPTION:
# Install `factory_runtime_probe` binary and its related libraries as a factory
# resource.
cros-factory-board_install_factory_runtime_probe() {
	local archive_dir="${WORKDIR}/factory_runtime_probe_archive"
	mkdir -p "${archive_dir}" || die
	"${SYSROOT}/usr/bin/factory_runtime_probe_installer" \
		--target "${archive_dir}" || die
	factory_create_resource "factory-runtime_probe" \
		"${archive_dir}" "bin/factory_runtime_probe" "."
}

# @FUNCTION: cros-factory-board_install_board_overlay
# @USAGE:
# @DESCRIPTION:
# Install board config into the toolkit and the factory bundle.
cros-factory-board_install_board_overlay() {
	if [[ -d "${FILESDIR}" ]]; then
		einfo "Creating ebuild-source-board-overlay.tar ..."
		factory_create_resource "ebuild-source-board-overlay" "${FILESDIR}" "." "."
	else
		einfo "FILESDIR does not exist. Skip creating ebuild-source-board-overlay.tar"
	fi
}

cros-factory-board_src_install() {
	cros-factory-board_install_board_overlay
	cros-factory-board_install_project_config
	cros-factory-board_install_project_overlay
	if ! use racc; then
		cros-factory-board_install_factory_runtime_probe
	fi
}

EXPORT_FUNCTIONS src_install
