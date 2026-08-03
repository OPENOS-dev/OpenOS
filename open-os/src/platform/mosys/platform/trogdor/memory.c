/* Copyright 2020 The ChromiumOS Authors
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "lib/fdt.h"
#include "lib/nonspd.h"

#include "mosys/platform.h"

#include "trogdor.h"

#define MAX_LINE_SIZE 128

static int trogdor_dimm_count;
static const struct nonspd_mem_info *mem_info;

enum trogdor_ram_manufacturer { SAMSUNG = 1, HYNIX = 6, MICRON = 255 };

/*
 * TODO(b/177917361) Remove once we have a more reliable API to probe
 * the memory information in Trogdor platform.
 *
 * get_ram_manufacturer_id - Get RAM manufacturer id from firmware log
 *
 * returns 0 to indicate success, <0 to indicate failure.
 */
static int get_ram_manufacturer_id(uint32_t *manufacturer_id)
{
	FILE *fp;
	char line[MAX_LINE_SIZE];

	fp = fopen("/sys/firmware/log", "r");

	if (!fp) {
		lprintf(LOG_ERR, "%s: Failed to open firmware log file.",
			__func__);
		return -1;
	}
	while (fgets(line, MAX_LINE_SIZE, fp)) {
		if (sscanf(line, "B - %*d - Manufacturer ID = %x",
			   manufacturer_id) == 1) {
			fclose(fp);
			return 0;
		}
	}

	fclose(fp);
	return -1;
}

static int populate_ram_info(void)
{
	static int done;
	static int ret;

	uint32_t ram_code;
	uint32_t ram_mid;

	if (!done) {
		if (fdt_get_ram_code(&ram_code) < 0) {
			lprintf(LOG_ERR, "%s: Unable to obtain RAM code.\n",
				__func__);
			return -1;
		}

		if (get_ram_manufacturer_id(&ram_mid) < 0) {
			lprintf(LOG_ERR,
				"%s: Unable to obtain RAM manufacturer id.\n",
				__func__);
			return -1;
		}

		trogdor_dimm_count = 1;

		if (ram_code == 1) {
			mem_info = &micron_lpddr4x_mt53d1g32d4dt_046wtd;
		} else if (ram_code == 2 && ram_mid == MICRON) {
			mem_info = &micron_lpddr4x_mt53e1g32d2np_046wta;
		} else if (ram_code == 2 && ram_mid == HYNIX) {
			mem_info = &hynix_lpddr4x_h9hcnnncpmmlxr_nee;
		} else if (ram_code == 2 && ram_mid == SAMSUNG) {
			mem_info = &samsung_lpddr4x_k4ube3d4aa_mgcr;
		} else if (ram_code == 3 && ram_mid == MICRON) {
			mem_info = &micron_lpddr4x_mt53e2g32d4nq_046wta;
		} else if (ram_code == 3 && ram_mid == HYNIX) {
			mem_info = &hynix_lpddr4x_h9hcnnnfammlxr_nee;
		} else if (ram_code == 4 && ram_mid == MICRON) {
			mem_info = &foresee_lpddr4x_flxc2004g_n1;
		} else if (ram_code == 4 && ram_mid == HYNIX) {
			mem_info = &hynix_lpddr4x_h54g56cyrbx247n;
		} else if (ram_code == 4 && ram_mid == SAMSUNG) {
			mem_info = &samsung_lpddr4x_k4ube3d4ab_mgcl;
		} else if (ram_code == 5 && ram_mid == HYNIX) {
			mem_info = &hynix_lpddr4x_h54g68cyrbx248n;
		} else if (ram_code == 5 && ram_mid == SAMSUNG) {
			mem_info = &samsung_lpddr4x_k4uce3q4ab_mgcl;
		} else {
			lprintf(LOG_ERR,
				"%s: Unknown ram code or "
				"manufacturer id.\n",
				__func__);
			ret = -1;
		}

		done = 1;
	}

	return ret;
}

/*
 * dimm_count  -  return total number of dimm slots
 *
 * @intf:       platform interface
 *
 * returns dimm slot count
 */
static int dimm_count(struct platform_intf *intf)
{
	if (populate_ram_info() < 0)
		return -1;

	return trogdor_dimm_count;
}

static int get_mem_info(struct platform_intf *intf, int dimm,
			const struct nonspd_mem_info **info)
{
	if (populate_ram_info() < 0)
		return -1;

	*info = mem_info;
	return 0;
}

struct memory_cb trogdor_memory_cb = {
	.dimm_count = &dimm_count,
	.nonspd_mem_info = &get_mem_info,
};
