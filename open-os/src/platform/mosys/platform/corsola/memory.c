/* Copyright 2022 The ChromiumOS Authors
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "lib/fdt.h"
#include "lib/nonspd.h"
#include "mosys/log.h"
#include "mosys/platform.h"

#include "corsola.h"

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
		*info = &hynix_lpddr4x_h9hcnnncpmmlxr_nee;
		break;
	case 0x01:
		*info = &samsung_lpddr4x_k4ube3d4ab_mgcl;
		break;
	case 0x02:
		*info = &micron_lpddr4x_mt53e1g32d2np_046wtb;
		break;
	case 0x03:
		*info = &hynix_lpddr4x_h54g56cyrbx247;
		break;
	case 0x04:
		*info = &cxmt_lpddr4x_cxdb5ccbm_ml_a;
		break;
	case 0x05:
		*info = &nanya_lpddr4x_nt6ap1024f32bl_j1;
		break;
	case 0x06:
		*info = &rayson_lpddr4x_rs1g32lr4d2bnr_46bt;
		break;
	case 0x11:
		*info = &hynix_lpddr4x_h9hcnnnbkmmlxr_nee;
		break;
	case 0x12:
		*info = &samsung_lpddr4x_k4u6e3s4aa_mgcr;
		break;
	case 0x13:
		*info = &micron_lpddr4x_mt53e512m32d1np_046wtb;
		break;
	case 0x20:
		*info = &micron_lpddr4x_mt53e2g32d4nq_046wtc;
		break;
	case 0x21:
		*info = &hynix_lpddr4x_h54g68cyrbx248;
		break;
	case 0x22:
		*info = &samsung_lpddr4x_k4uce3q4ab_mgcl;
		break;
	case 0x23:
		*info = &samsung_lpddr4x_k4uce3d4aa_mgcl;
		break;
	case 0x24:
		*info = &rayson_lpddr4x_rs2g32lv4d4bdt_46bt;
		break;
	case 0x25:
		*info = &rayson_lpddr4x_rs2g32lr4d4bnr_46bt;
		break;
	default:
		lprintf(LOG_ERR, "Invalid RAM code: %d.\n", ram_code);
		return -1;
	}
	return 0;
}

struct memory_cb corsola_memory_cb = {
	.dimm_count = dimm_count,
	.nonspd_mem_info = get_mem_info,
};
