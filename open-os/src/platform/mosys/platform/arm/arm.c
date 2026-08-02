/* Copyright 2024 The ChromiumOS Authors
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <stddef.h>

#include "mosys/command_list.h"
#include "mosys/platform.h"

#include "arm.h"

static struct platform_cmd *subcommands[] = {
	&cmd_memory,
	NULL,
};

static struct platform_cb generic_arm_cb = {
	.memory = &arm_memory_cb,
};

struct platform_intf platform_intf = {
	.sub = subcommands,
	.cb = &generic_arm_cb,
};
