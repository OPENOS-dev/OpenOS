/* Copyright 2020 The ChromiumOS Authors
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <stdlib.h>
#include <unistd.h>

#include "lib/file.h"
#include "mosys/command_list.h"
#include "mosys/platform.h"

#include "asurada.h"

static struct platform_cmd *asurada_sub[] = {
	&cmd_memory,
	NULL,
};

static struct platform_cb asurada_cb = {
	.memory = &asurada_memory_cb,
};

struct platform_intf platform_intf = {
	.sub = asurada_sub,
	.cb = &asurada_cb,
};
