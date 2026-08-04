#!/bin/sh
# Copyright 2024 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# The Realtek NIC on most puff models has a bug that causes system hangs
# when the L1 ASPM state is enabled.  However, the system cannot enter
# s0ix unless the PCIPM-L1.1 substate is enabled. Technically this is
# invalid, but older kernels allowed it.  To support the 5.15 uprev,
# we disable L1 and then manually set the L1.1 substate flag here using
# udev rules.

enable_dev_pcipm_l1() {
        device=$1

        # Extract L1SS base address and CTL1 offset
        l1ss_base=$(lspci -vvv -s "${device}" | awk '/L1 PM Substates/ { print $2 }' | sed 's/\[//g')
        PCI_L1SS_CTL1=$(printf "0x%X" $(( 0x${l1ss_base} + 8 )))

        # New register value, set bit 2, output as hex
        regval=$(setpci -s "${device}" "${PCI_L1SS_CTL1}.l")
        regval_new=$(printf "0x%X" $(( 0x${regval} | 2 )))

        # Write new value
        setpci -s "${device}" "${PCI_L1SS_CTL1}".l="${regval_new}"
}

# Realtek NIC
# Disable ASPM L1 and all substates first
echo 0 > "/sys/bus/pci/devices/0000:01:00.0/link/l1_aspm"

# Re-enable PCIPM L1.1 substate
enable_dev_pcipm_l1 01:00.0

# Associated PCI bridge
enable_dev_pcipm_l1 00:1c.0
