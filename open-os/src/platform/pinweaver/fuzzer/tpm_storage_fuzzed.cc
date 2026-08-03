/* Copyright 2021 The ChromiumOS Authors
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <stdbool.h>
#include <string.h>

#include "fuzzer_provider.h"
#include "pinweaver_eal.h"

extern "C" {

int pinweaver_eal_derive_keys(struct merkle_tree_t *merkle_tree)
{
	auto wrap_key = g_data_provider->ConsumeBytes<char>(
		sizeof(merkle_tree->wrap_key));
	wrap_key.resize(sizeof(merkle_tree->wrap_key));
	memcpy(merkle_tree->wrap_key, wrap_key.data(),
	       sizeof(merkle_tree->wrap_key));

	auto hmac_key = g_data_provider->ConsumeBytes<char>(
		sizeof(merkle_tree->hmac_key));
	hmac_key.resize(sizeof(merkle_tree->hmac_key));
	memcpy(merkle_tree->hmac_key, hmac_key.data(),
	       sizeof(merkle_tree->hmac_key));
	if (g_data_provider->ConsumeBool())
		return g_data_provider->ConsumeIntegral<int>();
	return 0;
}

int pinweaver_eal_storage_start()
{
	if (g_data_provider->ConsumeBool())
		return g_data_provider->ConsumeIntegral<int>();
	return 0;
}

int pinweaver_eal_storage_init_state(uint8_t *root_hash,
				     uint32_t *restart_count)
{
	auto rh = g_data_provider->ConsumeBytes<char>(PW_HASH_SIZE);
	rh.resize(PW_HASH_SIZE);
	memcpy(root_hash, rh.data(), PW_HASH_SIZE);

	*restart_count = g_data_provider->ConsumeIntegral<uint32_t>();

	if (g_data_provider->ConsumeBool())
		return g_data_provider->ConsumeIntegral<int>();
	return 0;
}

int pinweaver_eal_storage_get_log(struct pw_log_storage_t *dest)
{
	auto log = g_data_provider->ConsumeBytes<char>(
		sizeof(struct pw_log_storage_t));
	log.resize(sizeof(struct pw_log_storage_t));
	memcpy(dest, log.data(), sizeof(struct pw_log_storage_t));

	if (g_data_provider->ConsumeBool())
		return g_data_provider->ConsumeIntegral<int>();
	return 0;
}

int pinweaver_eal_storage_set_log(const struct pw_log_storage_t *log)
{
	if (g_data_provider->ConsumeBool())
		return g_data_provider->ConsumeIntegral<int>();
	return 0;
}

int pinweaver_eal_storage_get_tree_data(struct pw_long_term_storage_t *dest)
{
	auto tree = g_data_provider->ConsumeBytes<char>(
		sizeof(struct pw_long_term_storage_t));
	tree.resize(sizeof(struct pw_long_term_storage_t));
	memcpy(dest, tree.data(), sizeof(struct pw_long_term_storage_t));

	if (g_data_provider->ConsumeBool())
		return g_data_provider->ConsumeIntegral<int>();
	return 0;
}

int pinweaver_eal_storage_set_tree_data(
	const struct pw_long_term_storage_t *data)
{
	if (g_data_provider->ConsumeBool())
		return g_data_provider->ConsumeIntegral<int>();
	return 0;
}

int pinweaver_eal_storage_initialize_owner()
{
	if (g_data_provider->ConsumeBool())
		return g_data_provider->ConsumeIntegral<int>();
	return 0;
}

int pinweaver_eal_storage_get_ba_pk(uint8_t auth_channel,
			     struct pw_ba_pk_t *pk)
{
	auto key = g_data_provider->ConsumeBytes<char>(
		sizeof(struct pw_ba_pk_t));
	key.resize(sizeof(struct pw_ba_pk_t));
	memcpy(pk, key.data(), sizeof(struct pw_ba_pk_t));

	if (g_data_provider->ConsumeBool())
		return PW_ERR_BIO_AUTH_PK_NOT_ESTABLISHED;
	if (g_data_provider->ConsumeBool())
		return g_data_provider->ConsumeIntegral<int>();
	return 0;
}

int pinweaver_eal_storage_set_ba_pk(uint8_t auth_channel,
			     const struct pw_ba_pk_t *pk)
{
	if (g_data_provider->ConsumeBool())
		return g_data_provider->ConsumeIntegral<int>();
	return 0;
}
}
