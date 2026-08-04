#!/bin/bash

# Copyright 2016 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# All kernel command line changes must update the security base lines in
# the signer.  It rejects any settings it does not recognize and breaks the
# build.  So any modify_kernel_command_line() function change here needs to be
# reflected in ensure_secure_kernelparams.config.

# See crrev.com/i/216896 as an example.

# Removes any existing chromeos_pstore.ecc_size parameter from the given file
# and appends a new one with the provided value.
#   $1: Path to the config file to modify.
#   $2: The new value for chromeos_pstore.ecc_size.
update_pstore_ecc_size() {
  local file="$1"
  local new_value="$2"

  # First, remove the ecc_size parameter if it exists.
  # The `d` command in deletes the matching line.
  # This command does nothing if the line is not found.
  sed -i "/^chromeos_pstore.ecc_size=/d" "${file}"

  # Then, append the new ecc_size parameter.
  echo "chromeos_pstore.ecc_size=${new_value}" >> "${file}"
}

modify_kernel_command_line() {
  # Enable S0ix validation check in kernel
  echo "intel_idle.slp_s0_check=1" >> "$1"

  # Enable S0ix logging using GSMI
  echo "gsmi.s0ix_logging_enable=1" >> "$1"

  # Setup S0ix validation initial timeout for slp_s0_check
  echo "intel_idle.slp_s0_seed=15" >> "$1"

  # Don't disable the ability to run VMs.
  echo "disablevmx=off" >> "$1"

  # Enable l1d_flush for untrusted VM security
  echo "kvm-intel.vmentry_l1d_flush=always" >> "$1"

  # Disable framebuffer compression
  echo "i915.enable_fbc=0" >> "$1"

  # Set ECC parity data size to 16 bytes.
  # See b/413292019 and go/cros-ecc-rollout for more details.
  update_pstore_ecc_size "$1" "16"
}
