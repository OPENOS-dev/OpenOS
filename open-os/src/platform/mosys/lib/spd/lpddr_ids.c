/* Copyright 2022 The ChromiumOS Authors
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <stdbool.h>

#include "lib/lpddr_ids.h"

#include "lib/fdt.h"
#include "lib/nonspd.h"
#include "lib/nonspd_modules.h"
#include "mosys/mosys.h"

enum herobrine_ram_manufacturer {
	SAMSUNG = 0x01,
	HYNIX = 0x06,
	MICRON = 0xff,
};

const static struct {
	uint8_t manufacturer;
	uint8_t revision1;
	uint8_t revision2;
	const struct nonspd_mem_info *mem_info;
} ddr_table[] = {
	/* Current used components */
	{ HYNIX, 0x06, 0x00, &hynix_lpddr4x_h9hcnnnfammlxr_nee },
	{ MICRON, 0x07, 0x07, &micron_lpddr4x_mt53e2g32d4nq_046wtc },

	/* Placeholder for other AVL components in DLM */
	{ HYNIX, 0x00, 0x00, &hynix_lpddr4x_h54g68cyrbx248 },
	{ SAMSUNG, 0x00, 0x00, &samsung_lpddr4x_k4uce3q4ab_mgcl },
	{ SAMSUNG, 0x08, 0x00, &samsung_lpddr4x_k4ube3d4ab_mgcl },
};

const struct nonspd_mem_info *lpddr_info_from_fdt_ids(void)
{
	static const struct nonspd_mem_info *lpddr_mem_info;

	static bool ran_once;

	uint8_t ram_manufacturer;
	uint8_t ram_revision1;
	uint8_t ram_revision2;

	if (ran_once)
		return lpddr_mem_info;

	ran_once = true;

	if (fdt_get_ram_ids(&ram_manufacturer, &ram_revision1, &ram_revision2) <
	    0) {
		lprintf(LOG_ERR,
			"%s: Unable to obtain RAM manufacturer_id or revision_id.\n",
			__func__);
		return lpddr_mem_info;
	}

	for (int i = 0; i < ARRAY_SIZE(ddr_table); i++) {
		if ((ddr_table[i].manufacturer == ram_manufacturer) &&
		    (ddr_table[i].revision1 == ram_revision1) &&
		    (ddr_table[i].revision2 == ram_revision2)) {
			lpddr_mem_info = ddr_table[i].mem_info;
			return lpddr_mem_info;
		}
	}
	lprintf(LOG_ERR,
		"%s: Unrecognized configuration: Manufacturer ID: %#02x "
		"Revision ID: %#02x %#02x\n",
		__func__, ram_manufacturer, ram_revision1, ram_revision2);

	return lpddr_mem_info;
}
