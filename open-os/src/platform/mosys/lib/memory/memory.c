/* Copyright 2019 The ChromiumOS Authors
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "lib/memory.h"
#include "lib/nonspd.h"
#include "lib/smbios.h"

struct memory_cb smbios_memory_cb = {
	.dimm_count = smbios_dimm_count,
	.nonspd_mem_info = &spd_set_nonspd_info_from_smbios,
};
