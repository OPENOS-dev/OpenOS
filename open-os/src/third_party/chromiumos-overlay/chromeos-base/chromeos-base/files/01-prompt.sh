# Copyright 2026 OCS (Open Code Studio)
# License: GPL-3.0
# shellcheck shell=bash

# OPENOS: Replace \h with OPENOS-${model}-rev${board_id}.
# We do everything in a subshell in order to not leak variables to the shell.
# A special file "/run/dont-modify-ps1-for-testing" can be created for on-device
# tests which require a stable PS1 string.

if [[ ! -f /run/dont-modify-ps1-for-testing ]]; then
  PS1="$(
    model="$(cat /var/lib/openos/device-model 2>/dev/null || cat /run/chromeos-config/v1/name 2>/dev/null || hostname)"
    board_id="$(crossystem board_id 2>/dev/null || true)"

    hostname="OPENOS-${model}"
    if [[ -n "${board_id}" ]]; then
      hostname+="-rev${board_id}"
    fi

    echo "${PS1//\\h/${hostname}}"
  )"
fi
