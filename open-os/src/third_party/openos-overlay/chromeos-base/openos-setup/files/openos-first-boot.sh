#!/bin/sh
# Copyright 2026 OCS (Open Code Studio)
# Distributed under the terms of the GNU General Public License v3
#
# OPENOS First-Boot Setup Script
# Runs once on first boot (and after system updates that clear the flag).
# Idempotent — safe to run multiple times.

set -euo pipefail

JOB="openos-first-boot"
COMPLETED_FLAG="/mnt/stateful_partition/.openos_boot_completed"
OOBE_COMPLETED="/home/chronos/.oobe_completed"
OPT_CONFIG_DIR="/home/chronos/.config/opt"
OPT_REPOS_CONF="/etc/opt/opt-repos.conf"
STATE_PART="/mnt/stateful_partition"

log() {
    logger -t "${JOB}" "$@"
    echo "[${JOB}] $@"
}

# ─────────────────────────────────────────────────────────────
# 1. Skip ChromeOS OOBE — mark as completed so Chrome boots
#    directly to the login screen without Google setup wizard.
# ─────────────────────────────────────────────────────────────
skip_oobe() {
    if [ ! -f "${OOBE_COMPLETED}" ]; then
        log "Marking OOBE as completed — skipping Google setup wizard"
        mkdir -p "$(dirname "${OOBE_COMPLETED}")"
        touch "${OOBE_COMPLETED}"
        chown chronos:chronos "${OOBE_COMPLETED}"
    fi
}

# ─────────────────────────────────────────────────────────────
# 2. OPENOS System Identity — ensure branding is applied even
#    on stateful partition where it might be stale from OEM.
# ─────────────────────────────────────────────────────────────
apply_system_identity() {
    # LSB release: ensure OPENOS identity is written
    local lsb_file="${STATE_PART}/etc/lsb-release"
    if [ -f "${lsb_file}" ]; then
        # Replace any remaining ChromeOS/ChromiumOS references in stateful lsb
        if grep -q "CHROMEOS\|Chromium OS\|Google" "${lsb_file}" 2>/dev/null; then
            log "Patching stateful lsb-release for OPENOS branding"
            sed -i \
                -e 's/CHROMEOS_RELEASE_NAME=.*/CHROMEOS_RELEASE_NAME=OPENOS/' \
                -e 's/GOOGLE_RELEASE=.*/GOOGLE_RELEASE=/' \
                -e 's/DEVICETYPE=.*/DEVICETYPE=OPENOS/' \
                "${lsb_file}"
        fi
    fi

    # Hostname: set default OPENOS hostname if not already customized
    local hostname_file="${STATE_PART}/etc/hostname"
    if [ ! -f "${hostname_file}" ]; then
        echo "openos" > "${hostname_file}"
        log "Set default hostname: openos"
    fi
}

# ─────────────────────────────────────────────────────────────
# 3. OPT Package Manager — initialize config for first use.
#    The Rust binary (RepoManager::new()) already handles this,
#    but we pre-create the system-level repo config so it works
#    for all users and during early boot.
# ─────────────────────────────────────────────────────────────
init_opt_package_manager() {
    if [ ! -f "${OPT_REPOS_CONF}" ]; then
        log "Initializing OPT package manager configuration"
        mkdir -p "$(dirname "${OPT_REPOS_CONF}")"
        cat > "${OPT_REPOS_CONF}" << 'EOF'
# OPT Package Manager — Repository Configuration
# Managed by openos-first-boot, edit via: opt repo add/remove

[repo.openos-community]
type = "opt"
url = "https://open-code-studio.github.io/openos-repo"
enabled = true

[repo.debian-bookworm]
type = "apt"
url = "https://mirrors.tuna.tsinghua.edu.cn/debian"
suite = "bookworm"
components = ["main", "contrib", "non-free", "non-free-firmware"]
enabled = true
EOF
    fi

    # Ensure per-user OPT cache dir exists for chronos
    if [ ! -d "${OPT_CONFIG_DIR}" ]; then
        log "Creating per-user OPT config directory"
        mkdir -p "${OPT_CONFIG_DIR}"
        chown -R chronos:chronos "${OPT_CONFIG_DIR}"
    fi
}

# ─────────────────────────────────────────────────────────────
# 4. F-Droid Setup — ensure privileged extension has correct
#    permissions if the fdroid package is installed.
# ─────────────────────────────────────────────────────────────
setup_fdroid() {
    local fdroid_ext="/usr/share/fdroid-privileged-extension"
    if [ -d "${fdroid_ext}" ]; then
        log "F-Droid privileged extension detected, verifying setup"
        # Ensure permissions are correct for ARCVM access
        chmod 644 "${fdroid_ext}"/*.apk 2>/dev/null || true
    fi
}

# ─────────────────────────────────────────────────────────────
# 5. Cleanup Google residue — remove any Google-specific state
#    that may have been seeded into the stateful partition.
# ─────────────────────────────────────────────────────────────
cleanup_google_residue() {
    # Remove Google account state if any was seeded
    local google_state="${STATE_PART}/unencrypted/google-accounts"
    if [ -d "${google_state}" ]; then
        log "Removing Google account state from stateful partition"
        rm -rf "${google_state}"
    fi

    # Clean Chrome metrics consent file (we don't do metrics)
    local consent_file="${STATE_PART}/unencrypted/preserve/metrics-consent"
    if [ -f "${consent_file}" ]; then
        rm -f "${consent_file}"
    fi
}

# ─────────────────────────────────────────────────────────────
# 6. Developer-friendly defaults
# ─────────────────────────────────────────────────────────────
apply_developer_defaults() {
    # Enable SSH server key generation if not done
    local ssh_host_key="/etc/ssh/ssh_host_rsa_key"
    if [ ! -f "${ssh_host_key}" ] && [ -x /usr/bin/ssh-keygen ]; then
        log "Generating SSH host keys"
        /usr/bin/ssh-keygen -A 2>/dev/null || true
    fi
}

# ─────────────────────────────────────────────────────────────
# Main — run all setup steps, then mark completed.
# ─────────────────────────────────────────────────────────────
main() {
    log "OPENOS first-boot setup starting ..."

    skip_oobe
    apply_system_identity
    init_opt_package_manager
    setup_fdroid
    cleanup_google_residue
    apply_developer_defaults

    # Mark first-boot as completed so we don't run again
    touch "${COMPLETED_FLAG}"
    log "OPENOS first-boot setup complete — flag set at ${COMPLETED_FLAG}"
}

main "$@"
