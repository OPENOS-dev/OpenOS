/* Copyright 2021 The ChromiumOS Authors
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/*
 * Most modern x86 developments can be supported by common code which
 * assumes the following:
 *
 * - Memory information is available in SMBIOS
 *
 * This implements a common platform_intf for all devices which that
 * applies to.
 */

#include <stdlib.h>

#include "lib/memory.h"
#include "lib/smbios.h"
#include "mosys/command_list.h"
#include "mosys/platform.h"

static struct platform_cmd *subcommands[] = {
	&cmd_memory,
	NULL,
};

static struct platform_cb generic_x86_cb = {
	.memory = &smbios_memory_cb,
};

struct platform_intf platform_intf = {
	.sub = subcommands,
	.cb = &generic_x86_cb,
};
