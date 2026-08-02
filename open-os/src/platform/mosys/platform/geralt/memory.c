/* Copyright 2023 The ChromiumOS Authors
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "lib/fdt.h"
#include "lib/nonspd.h"
#include "mosys/log.h"
#include "mosys/platform.h"

#include "geralt.h"

#define DIMM_COUNT 2

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
		*info = &samsung_lpddr4x_k4ube3d4ab_mgcl;
		break;
	case 0x01:
		*info = &micron_lpddr4x_mt53e1g32d2np_046wtb;
		break;
	case 0x02:
		*info = &hynix_lpddr4x_h54g56cyrbx247;
		break;
	case 0x10:
		*info = &hynix_lpddr4x_h9hcnnnbkmmlxr_nee;
		break;
	case 0x11:
		*info = &samsung_lpddr4x_k4u6e3s4ab_mgcl;
		break;
	case 0x12:
		*info = &micron_lpddr4x_mt53e512m32d1np_046wtb;
		break;
	case 0x13:
		*info = &hynix_lpddr4x_h54g46cyrbx267;
		break;
	default:
		lprintf(LOG_ERR, "Invalid RAM code: %d.\n", ram_code);
		return -1;
	}
	return 0;
}

struct memory_cb geralt_memory_cb = {
	.dimm_count = dimm_count,
	.nonspd_mem_info = get_mem_info,
};
