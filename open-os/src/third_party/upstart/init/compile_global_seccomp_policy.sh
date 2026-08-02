#!/bin/bash
# Copyright 2021 The Chromium OS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Used to generate the compiled seccomp policy from the text-based policy.
# Requires a text-based policy to be present in the same dir as the script.

echo "compiling global seccomp policy"
compile_seccomp_policy --arch-json "$1" \
	--denylist global_seccomp.policy global_seccomp_policy
${LD} -r -b binary global_seccomp_policy -o global_seccomp_policy.o
