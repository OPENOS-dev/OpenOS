/* Copyright 2021 The ChromiumOS Authors
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <stdbool.h>
#include <string.h>

#include "pinweaver_eal.h"

int pinweaver_eal_derive_keys(struct merkle_tree_t *merkle_tree)
{
	/* TODO */
	return 0;
}

int pinweaver_eal_storage_start(void)
{
	/* TODO */
	return 0;
}

int pinweaver_eal_storage_init_state(uint8_t *root_hash,
				     uint32_t *restart_count)
{
	/* TODO */
	*restart_count = 0;
	return 0;
}

int pinweaver_eal_storage_get_log(struct pw_log_storage_t *dest)
{
	/* TODO */
	return 0;
}

int pinweaver_eal_storage_set_log(const struct pw_log_storage_t *log)
{
	/* TODO */
	return 0;
}

int pinweaver_eal_storage_get_tree_data(struct pw_long_term_storage_t *dest)
{
	/* TODO */
	return 0;
}

int pinweaver_eal_storage_set_tree_data(
		const struct pw_long_term_storage_t *data)
{
	/* TODO */
	return 0;
}

int pinweaver_eal_storage_initialize_owner()
{
	/* TODO */
	return 0;
}
