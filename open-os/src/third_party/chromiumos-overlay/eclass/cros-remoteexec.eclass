# Copyright 2024 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# @ECLASS: cros-remoteexec.eclass
# @MAINTAINER:
# ChromiumOS Build Team
# @BUGREPORTS:
# Please report bugs via http://crbug.com/new (with label Build)
# @VCSURL: https://chromium.googlesource.com/chromiumos/overlays/chromiumos-overlay/+/HEAD/eclass/@ECLASS@
# @BLURB: Eclass to help building packages with remoteexec.

inherit cros-constants cros-credentials

# @ECLASS-VARIABLE: CROS_REMOTEEXEC_LOG_DIR
# @DESCRIPTION: Path to the log output directory used by reclient.
: "${CROS_REMOTEEXEC_LOG_DIR:=/tmp/reclient-${CATEGORY}_${PF}}"

# @ECLASS-VARIABLE: CROS_REMOTEEXEC_RECLIENT_PATH
# @DESCRIPTION: Directory containing the reclient binaries.
: "${CROS_REMOTEEXEC_RECLIENT_PATH:="${DEPOT_TOOLS}/.cipd_bin/reclient"}"

# @ECLASS-VARIABLE: CROS_REMOTEEXEC_REWRAPPER_PATH
# @DESCRIPTION: Path to the rewrapper executable. Ebuilds can use this variable to run compilers with rewrapper.
export CROS_REMOTEEXEC_REWRAPPER_PATH="${CROS_REMOTEEXEC_RECLIENT_PATH}/rewrapper"

# @ECLASS-VARIABLE: CROS_REMOTEEXEC_DEBUG
# @DESCRIPTION: Set to 1 or true to enable verbose debug.
: "${CROS_REMOTEEXEC_DEBUG:=}"

# @ECLASS-VARIABLE: CROS_REMOTEEXEC_DOWNLOAD_TEMP_DIR
# @DESCRIPTION: Directory to store temporary downloads. Must be on the same
# filesystem as your build artifacts. If empty CWD is used.
: "${CROS_REMOTEEXEC_DOWNLOAD_TEMP_DIR:=}"

# Checks if remoteexec should be used.
cros-remoteexec_use_remoteexec() {
	[[ -n "${USE_REMOTEEXEC}" && "${USE_REMOTEEXEC}" == "true" && \
		"$(cros-fetch_google_app_credentials)" == "true" ]]
}

# Checks if remoteexec_links should be used.
cros-remoteexec_use_remoteexec_links() {
	[[ -n "${USE_REMOTEEXEC_LINKS}" && "${USE_REMOTEEXEC_LINKS}" == "true" ]]
}

# Performs initialization to start the reproxy process.
#
# Args:
#   --disable-on-failure: If reclient fails to start, disable reclient support
#                         and continue.
cros-remoteexec_initialize() {
	if ! cros-remoteexec_use_remoteexec; then
		return 0
	fi
	local disable_on_failure=0
	if [[ $# -gt 0 && "$1" == "--disable-on-failure" ]]; then
		disable_on_failure=1
	fi

	einfo "!!!! Using reclient to speed up builds !!!!"

	# Startup reproxy. ============================================

	# 1. Use builder-populated ${REPROXY_CFG_FILE} value, or default for local builds.
	REPROXY_CFG_FILE=${REPROXY_CFG_FILE:-reproxy_experimental.cfg}
	einfo "Using ${REPROXY_CFG_FILE} config."

	# Set RBE_* environment variables to configure reclient.
	# See https://github.com/bazelbuild/reclient/blob/main/README.md
	export RBE_log_dir="${CROS_REMOTEEXEC_LOG_DIR}"
	export RBE_proxy_log_dir="${CROS_REMOTEEXEC_LOG_DIR}"
	export RBE_server_address="unix:///tmp/reproxy-${CATEGORY}_${PF}.sock"

	# 2. Bootstrap is used to startup reproxy. This is used to route local builds to RBE.
	mkdir -p "${CROS_REMOTEEXEC_LOG_DIR}" || die

	if [[ "${CROS_REMOTEEXEC_DEBUG}" == 1 || "${CROS_REMOTEEXEC_DEBUG}" == "true" ]]; then
		export RBE_log_format=text
		export RBE_v=10
	fi

	if [[ -n "${CROS_REMOTEEXEC_DOWNLOAD_TEMP_DIR}" ]]; then
		export RBE_download_tmp_dir="${CROS_REMOTEEXEC_DOWNLOAD_TEMP_DIR}"
	fi

	local -i tries
	einfo "Starting reproxy"
	for ((tries=3; tries > 0; tries--)); do
		if "${CROS_REMOTEEXEC_RECLIENT_PATH}/bootstrap" \
				--cfg="/mnt/host/source/chromite/sdk/reclient_cfgs/${REPROXY_CFG_FILE}" \
				--re_proxy "${CROS_REMOTEEXEC_RECLIENT_PATH}/reproxy"; then
			break
		elif [[ "${tries}" -gt 1 ]]; then
			einfo "Failed to start reproxy, retrying..."
			sleep 5
		else
			einfo "Failed to start reproxy"
		fi
	done

	if [[ "${tries}" -eq 0 ]]; then
		if [[ "${disable_on_failure}" -eq 1 ]]; then
			ewarn "reproxy failed to start."
			ewarn "Falling back to local only build."

			USE_REMOTEEXEC=false
			return 1
		else
			die "Fatal - cannot start reproxy for distributed builds."
		fi
	fi

	# Increase the number of parallel run to 30 * {number of processors}. Though, if it is too
	# large the performance gets slow down, so limit by 1000 empirically.
	local num_parallel=$(($(nproc) * 30))
	local j_limit=1000
	local parallelism=$((num_parallel < j_limit ? num_parallel : j_limit))
	export MAKEOPTS="${MAKEOPTS} -j${parallelism}"

	# Clean up if the build fails.
	register_die_hook cros-remoteexec_shutdown

	return 0
}

# Shuts down the reproxy process.
cros-remoteexec_shutdown() {
	if ! cros-remoteexec_use_remoteexec; then
		return 0
	fi

	einfo "Shutting down reproxy"

	"${CROS_REMOTEEXEC_RECLIENT_PATH}/bootstrap" --shutdown \
		--cfg="/mnt/host/source/chromite/sdk/reclient_cfgs/${REPROXY_CFG_FILE}" \
		--re_proxy "${CROS_REMOTEEXEC_RECLIENT_PATH}/reproxy"
	einfo "reclient log location: ${RBE_log_dir}"
}
