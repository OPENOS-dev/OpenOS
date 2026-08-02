/* Copyright 2022 The ChromiumOS Authors
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <stdlib.h>
#include <unistd.h>

#include "mosys/command_list.h"
#include "mosys/platform.h"

#include "corsola.h"

static struct platform_cmd *corsola_sub[] = {
	&cmd_memory,
	NULL,
};

static struct platform_cb corsola_cb = {
	.memory = &corsola_memory_cb,
};

struct platform_intf platform_intf = {
	.sub = corsola_sub,
	.cb = &corsola_cb,
};
