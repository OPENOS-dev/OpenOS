/* Copyright 2020 The ChromiumOS Authors
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "lib/fdt.h"
#include "lib/nonspd.h"
#include "mosys/log.h"
#include "mosys/platform.h"

#include "asurada.h"

#define DIMM_COUNT 1

/*
 * dimm_count  -  return total number of dimm slots
 *
 * @intf:       platform interface
 *
 * returns dimm slot count
 */
static int dimm_count(struct platform_intf *intf)
{
	return DIMM_COUNT;
}

static int get_mem_info(struct platform_intf *intf, int dimm,
			const struct nonspd_mem_info **info)
{
	uint32_t ram_code;

	if (fdt_get_ram_code(&ram_code) < 0) {
		lprintf(LOG_ERR, "Unable to obtain RAM code.\n");
		return -1;
	}

	switch (ram_code) {
	case 0x00:
		*info = &micron_lpddr4x_mt29vzzzbd9dqkpr_046;
		break;
	case 0x01:
		*info = &micron_lpddr4x_mt29vzzzad8gqfsl_046;
		break;
	case 0x02:
		*info = &samsung_lpddr4x_kmdp6001da_b425;
		break;
	case 0x03:
		*info = &samsung_lpddr4x_kmdv6001da_b620;
		break;
	case 0x40:
		*info = &micron_lpddr4x_mt53e1g32d2np_046wta;
		break;
	case 0x50:
		*info = &samsung_lpddr4x_k4ube3d4aa_mgcr;
		break;
	case 0x51:
		*info = &hynix_lpddr4x_h9hcnnncpmmlxr_nee;
		break;
	case 0x60:
		*info = &micron_lpddr4x_mt53e2g32d4nq_046wta;
		break;
	case 0x70:
		*info = &samsung_lpddr4x_k4uce3q4aa_mgcr;
		break;
	case 0x71:
		*info = &hynix_lpddr4x_h9hcnnnfammlxr_nee;
		break;
	default:
		lprintf(LOG_ERR, "Invalid RAM code: %d.\n", ram_code);
		return -1;
	}
	return 0;
}

struct memory_cb asurada_memory_cb = {
	.dimm_count = dimm_count,
	.nonspd_mem_info = get_mem_info,
};
