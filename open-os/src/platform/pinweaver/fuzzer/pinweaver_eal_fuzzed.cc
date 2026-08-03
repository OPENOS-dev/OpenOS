/* Copyright 2021 The ChromiumOS Authors
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <openssl/bn.h>
#include <openssl/ec.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <string.h>
#include <unistd.h>

#include "pinweaver_eal.h"
#include "pinweaver_eal_tpm.h"
#include "fuzzer_provider.h"

uint64_t g_time_base = 0;

extern "C" {
int pinweaver_eal_safe_memcmp(const void *s1, const void *s2, size_t len)
{
	const uint8_t *us1 = static_cast<const uint8_t *>(s1);
	const uint8_t *us2 = static_cast<const uint8_t *>(s2);
	int result = 0;

	while (len--)
		result ^= *us1++ ^ *us2++;

	return result & 1;
}

int pinweaver_eal_rand_bytes(void *buf, size_t size)
{
	if (g_data_provider->ConsumeBool())
		return -1;
	auto bytes = g_data_provider->ConsumeBytes<char>(size);
	bytes.resize(size);
	memcpy(buf, bytes.data(), size);
	return 0;
}

uint64_t pinweaver_eal_seconds_since_boot()
{
	if (g_data_provider->ConsumeIntegralInRange<char>(0, 7) == 0)
		g_time_base +=
			g_data_provider->ConsumeIntegralInRange<uint64_t>(0,
									  3600);
	return g_time_base;
}

int pinweaver_eal_sha256_init(pinweaver_eal_sha256_ctx_t *ctx)
{
	if (g_data_provider->ConsumeBool())
		return -1;

	int rv = SHA256_Init(ctx);
	if (rv != 1) {
		PINWEAVER_EAL_INFO("SHA256_Init failed: %d", rv);
	}
	return rv == 1 ? 0 : -1;
}

int pinweaver_eal_sha256_update(pinweaver_eal_sha256_ctx_t *ctx,
				const void *data, size_t size)
{
	if (g_data_provider->ConsumeBool())
		return -1;

	int rv = SHA256_Update(ctx, data, size);
	if (rv != 1) {
		PINWEAVER_EAL_INFO("SHA256_Update failed: %d", rv);
	}
	return rv == 1 ? 0 : -1;
}

int pinweaver_eal_sha256_final(pinweaver_eal_sha256_ctx_t *ctx, void *res)
{
	if (g_data_provider->ConsumeBool())
		return -1;

	int rv = SHA256_Final((unsigned char *)res, ctx);
	if (rv != 1) {
		PINWEAVER_EAL_INFO("SHA256_Final failed: %d", rv);
	}
	return rv == 1 ? 0 : -1;
}

int pinweaver_eal_hmac_sha256_init(pinweaver_eal_hmac_sha256_ctx_t *ctx,
				   const void *key,
				   size_t key_size /* in bytes */)
{
	if (g_data_provider->ConsumeBool())
		return -1;

	*ctx = HMAC_CTX_new();
	if (!*ctx) {
		PINWEAVER_EAL_INFO("HMAC_CTX_new failed");
		return -1;
	}
	int rv = HMAC_Init_ex(*ctx, key, key_size, EVP_sha256(), NULL);
	if (rv != 1) {
		PINWEAVER_EAL_INFO("HMAC_Init_ex failed: %d", rv);
	}
	return rv == 1 ? 0 : -1;
}

int pinweaver_eal_hmac_sha256_update(pinweaver_eal_hmac_sha256_ctx_t *ctx,
				     const void *data, size_t size)
{
	if (g_data_provider->ConsumeBool())
		return -1;

	int rv = HMAC_Update(*ctx, (const unsigned char *)data, size);
	if (rv != 1) {
		PINWEAVER_EAL_INFO("HMAC_Update failed: %d", rv);
	}
	return rv == 1 ? 0 : -1;
}

int pinweaver_eal_hmac_sha256_final(pinweaver_eal_hmac_sha256_ctx_t *ctx,
				    void *res)
{
	unsigned int len;
	// Free the memory to prevent leaking memory.
	int rv = HMAC_Final(*ctx, (unsigned char *)res, &len);
	HMAC_CTX_free(*ctx);
	*ctx = NULL;
	if (rv != 1) {
		PINWEAVER_EAL_INFO("HMAC_Final failed: %d", rv);
	}
	if (g_data_provider->ConsumeBool()) {
		return -1;
	}
	return rv == 1 ? 0 : -1;
}

int pinweaver_eal_aes256_ctr(const void *key, size_t key_size, /* in
								  bytes
								*/
			     const void *iv, const void *data, size_t size,
			     void *res)
{
	EVP_CIPHER_CTX *ctx;
	int rv;
	int len, len_final;

	if (g_data_provider->ConsumeBool())
		return -1;

	if (key_size != 256 / 8)
		return -1;
	ctx = EVP_CIPHER_CTX_new();
	if (!ctx)
		return -1;
	rv = EVP_EncryptInit(ctx, EVP_aes_256_ctr(), (const unsigned char *)key,
			     (const unsigned char *)iv);
	if (rv != 1)
		goto out;
	rv = EVP_EncryptUpdate(ctx, (unsigned char *)res, &len,
			       (const unsigned char *)data, size);
	if (rv != 1)
		goto out;
	rv = EVP_EncryptFinal(ctx, (unsigned char *)res + len, &len_final);
out:
	EVP_CIPHER_CTX_free(ctx);
	return rv == 1 ? 0 : -1;
}

int pinweaver_eal_aes256_ctr_custom(const void *key, size_t key_size,
				    const void *iv, const void *data, size_t size,
				    void *res)
{
	return pinweaver_eal_aes256_ctr(key, key_size, iv, data, size, res);
}

uint8_t pinweaver_eal_get_current_pcr_digest(const uint8_t bitmask[2],
					     uint8_t sha256_of_selected_pcr[32])
{
	if (g_data_provider->ConsumeBool())
		return -1;

	uint8_t pcr_value[SHA256_DIGEST_SIZE];
	/* TODO */
	memset(pcr_value, 0, 32);

	pinweaver_eal_sha256_ctx_t ctx;
	if (pinweaver_eal_sha256_init(&ctx))
		return -1;
	if (pinweaver_eal_sha256_update(&ctx, pcr_value, sizeof(pcr_value))) {
		pinweaver_eal_sha256_final(&ctx, sha256_of_selected_pcr);
		return -1;
	}
	return pinweaver_eal_sha256_final(&ctx, sha256_of_selected_pcr);
}

int pinweaver_eal_memcpy_s(void *dest, size_t destsz, const void *src,
			   size_t count)
{
	if (g_data_provider->ConsumeBool()) {
		return -1;
	}

	if (count == 0) {
		return 0;
	}

	if (dest == NULL) {
		abort();
	}

	if (src == NULL) {
		memset(dest, 0, destsz);
		abort();
	}

	if (destsz < count) {
		memset(dest, 0, destsz);
		abort();
	}

	memcpy(dest, src, count);
	return 0;
}

int pinweaver_eal_ecdh_derive(const struct pw_ba_ecc_pt_t *ecc_pt_in,
			     void *secret, size_t *secret_size,
			     struct pw_ba_ecc_pt_t *ecc_pt_out)
{
	auto sec = g_data_provider->ConsumeBytes<char>(*secret_size);
	*secret_size = sec.size();
	if (*secret_size != 0)
		memcpy(secret, sec.data(), *secret_size);

	auto ecc_pt = g_data_provider->ConsumeBytes<char>(
		sizeof(struct pw_ba_ecc_pt_t));
	ecc_pt.resize(sizeof(struct pw_ba_ecc_pt_t));
	memcpy(ecc_pt_out, ecc_pt.data(), sizeof(struct pw_ba_ecc_pt_t));

	if (g_data_provider->ConsumeBool())
		return g_data_provider->ConsumeIntegral<int>();
	return 0;
}
}
