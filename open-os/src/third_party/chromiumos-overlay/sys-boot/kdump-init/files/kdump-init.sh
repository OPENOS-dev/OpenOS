#!/bin/sh
# Copyright 2023 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# kdump-image is a symbolic link to the kernel image used by kexec.
# On arm this is a link to Image, and on X86 a link to vmlinuz.
# The kernel image contains a ramdisk doing all the kdump logic.
KDUMP_IMAGE=/usr/share/kdump/boot/kdump-image

# Only load kdump images in dev/test images.
if crossystem 'cros_debug?1'; then
  # Enable serial console for the ease of debugging.
  # TODO(b/453883589): kdump doesn't work without console enabled. Need to
  # figure out why.
  # TODO(b/451780016): Not all boards use ttyS0 as the console. May need to
  # update this when supporting more boards.
  kexec-lite -a LoadCrash -c "$(cat /proc/cmdline) console=ttyS0,115200n8" -k "${KDUMP_IMAGE}"
fi

sysctl -w kernel.kexec_load_disabled=1
