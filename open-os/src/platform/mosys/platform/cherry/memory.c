/* Copyright 2021 The ChromiumOS Authors
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "lib/fdt.h"
#include "lib/nonspd.h"
#include "mosys/log.h"
#include "mosys/platform.h"

#include "cherry.h"

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
		*info = &hynix_lpddr4x_h9hcnnncpmmlxr_nee;
		break;
	case 0x01:
		*info = &samsung_lpddr4x_k4ube3d4aa_mgcr;
		break;
	case 0x02:
		*info = &hynix_lpddr4x_h54g56cyrbx247;
		break;
	case 0x03:
		*info = &micron_lpddr4x_mt53e1g32d2np_046wtb;
		break;
	case 0x04:
		*info = &samsung_lpddr4x_k4ube3d4ab_mgcl;
		break;
	case 0x10:
		*info = &micron_lpddr4x_mt53e1g32d2np_046wta;
		break;
	case 0x20:
		*info = &micron_lpddr4x_mt53e2g32d4nq_046wta;
		break;
	case 0x30:
		*info = &samsung_lpddr4x_k4uce3q4aa_mgcr;
		break;
	case 0x31:
		*info = &hynix_lpddr4x_h9hcnnnfammlxr_nee;
		break;
	case 0x40:
		*info = &micron_lpddr4x_mt53e512m32d2np_046;
		break;
	case 0x41:
		*info = &samsung_lpddr4x_k4u6e3s4aa_mgcr;
		break;
	case 0x42:
		*info = &hynix_lpddr4x_h9hcnnnbkmmlxr_nee;
		break;
	case 0x43:
		*info = &micron_lpddr4x_mt53e512m32d1np_046wtb;
		break;
	case 0x44:
		*info = &samsung_lpddr4x_k4u6e3s4ab_mgcl;
		break;
	case 0x45:
		*info = &hynix_lpddr4x_h54g46cyrbx267;
		break;
	default:
		lprintf(LOG_ERR, "Invalid RAM code: %d.\n", ram_code);
		return -1;
	}
	return 0;
}

struct memory_cb cherry_memory_cb = {
	.dimm_count = dimm_count,
	.nonspd_mem_info = get_mem_info,
};
