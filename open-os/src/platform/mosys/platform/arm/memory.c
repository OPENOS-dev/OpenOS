/* Copyright 2024 The ChromiumOS Authors
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "lib/cbi.h"
#include "lib/fdt.h"
#include "lib/memory.h"
#include "lib/nonspd.h"

#include "arm.h"

/*
 * Original dimm_count is used to report the number of DIMMs plugged into the
 * DIMM sockets. For the LPDDR memory module, the package is different from DDR
 * SDRAM. We report the number of channels utilized by the memory modules
 * instead.
 */
static int dimm_count(struct platform_intf *intf)
{
	return fdt_channel_count();
}

static struct nonspd_mem_info mem_info = {
	.part_num = { 'U', 'N', 'P', 'R', 'O', 'V', 'I', 'S', 'I', 'O', 'N',
		      'E', 'D' },
};

static enum spd_dram_type map_to_spd_dram_type(uint8_t lpddr_type)
{
	switch (lpddr_type) {
	case 3:
		return SPD_DRAM_TYPE_LPDDR3;
	case 4:
		return SPD_DRAM_TYPE_LPDDR4;
	case 5:
		return SPD_DRAM_TYPE_LPDDR5;
	default:
		lprintf(LOG_ERR, "%s: Unknown memory type: %u\n", __func__,
			lpddr_type);
		return SPD_DRAM_TYPE_UNDEFINED;
	}
}

static int get_mem_info(struct platform_intf *intf, int dimm,
			const struct nonspd_mem_info **info)
{
	struct fdt_memory_channel chan_info;
	static bool get_part_num_once;

	if (fdt_get_channel_info(dimm, &chan_info))
		return -1;

	mem_info.dram_type = map_to_spd_dram_type(chan_info.lpddr_type);
	if (mem_info.dram_type == SPD_DRAM_TYPE_UNDEFINED)
		return -1;

	mem_info.module_mfg_id.lsb = chan_info.manufacturer_id;
	mem_info.revision[0] = chan_info.revision[0];
	mem_info.revision[1] = chan_info.revision[1];
	mem_info.module_size_mbits = chan_info.density_mbits;
	mem_info.num_ranks = chan_info.num_ranks;
	mem_info.device_width = chan_info.io_width;

	if (!get_part_num_once) {
		cbi_get_dram_part_num(mem_info.part_num,
				      sizeof(mem_info.part_num));
		get_part_num_once = true;
	}

	*info = &mem_info;

	return 0;
}

struct memory_cb arm_memory_cb = {
	.dimm_count = dimm_count,
	.nonspd_mem_info = get_mem_info,
};
