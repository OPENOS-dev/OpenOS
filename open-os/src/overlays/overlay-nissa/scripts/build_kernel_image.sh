#!/bin/bash

# Copyright 2021 The ChromiumOS Authors
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

  # Enable GuC loading
  echo "i915.enable_guc=3" >> "$1"

  # Check for S0ix failures and show warnings on failures
  echo "intel_pmc_core.warn_on_s0ix_failures=1" >> "$1"

  # Ensure internal devices are also in their own DMA domain
  echo "intel_iommu=on" >> "$1"

  # The 5G driver requires a lot of swiotlb buffers (b/201020414)
  # So increase the swiotlb slots from default 32768 (64MB) to 65536 (128MB)
  echo "swiotlb=65536" >> "$1"

  # Set ECC parity data size to 0 bytes and disable ECC.
  # See b/436373634.
  update_pstore_ecc_size "$1" "0"
}
