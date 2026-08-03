#!/bin/bash
# Copyright 2023 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# .bashrc read in by the borealis chroot shell

alias steam='steam -no-cef-sandbox'
alias vtest_start='./start_vtest.sh && export VK_ICD_FILENAMES=/usr/share/vulkan/icd.d/virtio_icd.x86_64.json && export VN_DEBUG=vtest'
alias vtest_stop='./stop_vtest.sh && unset VK_ICD_FILENAMES && unset VN_DEBUG'
