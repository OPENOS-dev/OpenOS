/* Copyright 2023 The ChromiumOS Authors
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <string.h>

#include <zephyr/fs/fcb.h>
#include <zephyr/logging/log.h>
#include <zephyr/settings/settings.h>

#include "drivers/console_util.h"
#include "sim_info.h"
#include "sim_persist.h"
#include "sim.h"
#include "sim_gpio.h"

LOG_MODULE_DECLARE(sim, LOG_LEVEL_DBG);

static const char *ID_PATH = "sim/id/%u";

void sim_setting_set_slot_id(int slot_number, int32_t *in)
{
	char key[32];

	snprintf(key, sizeof(key), ID_PATH, slot_number);

	if (in) {
		SIM_SAVE_FIELD(key, *in);
	} else {
		SIM_ERASE_FIELD(key);
	}
}

bool sim_setting_get_slot_id(int slot_number, int32_t *out)
{
	char key[32];

	snprintf(key, sizeof(key), ID_PATH, slot_number);

	struct storage_result result = INIT_STORAGE_RESULT(*out);
	bool valid = sim_persist_load(key, &result);

	if (!valid) {
		*out = 0;
	}
	return valid;
}

static const char *NAME_PATH = "sim/name/%u";

void sim_setting_set_slot_name(int slot_number, struct slot_name *in)
{
	char key[32];

	snprintf(key, sizeof(key), NAME_PATH, slot_number);

	if (in) {
		SIM_SAVE_FIELD(key, *in);
	} else {
		SIM_ERASE_FIELD(key);
	}
}

bool sim_setting_get_slot_name(int slot_number, struct slot_name *out)
{
	char key[32];

	snprintf(key, sizeof(key), NAME_PATH, slot_number);

	struct storage_result result = INIT_STORAGE_RESULT(*out);
	bool valid = sim_persist_load(key, &result);

	if (!valid) {
		out->name[0] = '\0';
	}
	out->name[sizeof(out->name) - 1] = '\0';
	return valid;
}
