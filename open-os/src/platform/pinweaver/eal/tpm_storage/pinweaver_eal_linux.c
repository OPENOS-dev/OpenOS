/* Copyright 2021 The ChromiumOS Authors
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <fcntl.h>
#include <openssl/bn.h>
#include <openssl/ec.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <string.h>
#include <sys/sysinfo.h>
#include <unistd.h>

#include "pinweaver_eal.h"
#include "pinweaver_eal_tpm.h"

static uint8_t g_tpm_key_hash_volatile[32] = {0,};
static bool g_tpm_key_hash_volatile_is_set = false;
static bool g_tpm_key_hash_is_committed = false;
static const char *kTpmKeyHashCommitPath = "/tmp/pw_tpm_key_hash";

static const char* get_openssl_error()
{
	return ERR_error_string(ERR_get_error(), NULL);
}

int pinweaver_eal_sha256_init(pinweaver_eal_sha256_ctx_t *ctx)
{
	int rv = SHA256_Init(ctx);
	if (rv != 1) {
		PINWEAVER_EAL_INFO("SHA256_Init failed: %d", rv);
	}
	return rv == 1 ? 0 : -1;
}

int pinweaver_eal_sha256_update(pinweaver_eal_sha256_ctx_t *ctx,
				const void *data,
				size_t size)
{
	int rv = SHA256_Update(ctx, data, size);
	if (rv != 1) {
		PINWEAVER_EAL_INFO("SHA256_Update failed: %d", rv);
	}
	return rv == 1 ? 0 : -1;
}

int pinweaver_eal_sha256_final(pinweaver_eal_sha256_ctx_t *ctx,
			       void *res)
{
	int rv = SHA256_Final(res, ctx);
	if (rv != 1) {
		PINWEAVER_EAL_INFO("SHA256_Final failed: %d", rv);
	}
	return rv == 1 ? 0 : -1;
}

int pinweaver_eal_hmac_sha256_init(pinweaver_eal_hmac_sha256_ctx_t *ctx,
				   const void *key,
				   size_t key_size /* in bytes */)
{
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
				     const void *data,
				     size_t size)
{
	int rv = HMAC_Update(*ctx, data, size);
	if (rv != 1) {
		PINWEAVER_EAL_INFO("HMAC_Update failed: %d", rv);
	}
	return rv == 1 ? 0 : -1;
}

int pinweaver_eal_hmac_sha256_final(pinweaver_eal_hmac_sha256_ctx_t *ctx,
				    void *res)
{
	unsigned int len;
	int rv = HMAC_Final(*ctx, res, &len);
	HMAC_CTX_free(*ctx);
	*ctx = NULL;
	if (rv != 1) {
		PINWEAVER_EAL_INFO("HMAC_Final failed: %d", rv);
	}
	return rv == 1 ? 0 : -1;
}

int pinweaver_eal_aes256_ctr(const void *key,
			     size_t key_size, /* in bytes */
			     const void *iv,
			     const void *data,
			     size_t size,
			     void *res)
{
	EVP_CIPHER_CTX *ctx;
	int rv;
	int len, len_final;

	if (key_size != 256/8)
		return -1;
	ctx = EVP_CIPHER_CTX_new();
	if (!ctx)
		return -1;
	rv = EVP_EncryptInit(ctx, EVP_aes_256_ctr(), key, iv);
	if (rv != 1)
		goto out;
	rv = EVP_EncryptUpdate(ctx, res, &len, data, size);
	if (rv != 1)
		goto out;
	rv = EVP_EncryptFinal(ctx, res+len, &len_final);
out:
	EVP_CIPHER_CTX_free(ctx);
	return rv == 1 ? 0 : -1;
}

int pinweaver_eal_aes128_cfb(const void *key,
			     size_t key_size, /* in bytes */
			     const void *iv,
			     const void *data,
			     size_t size,
			     int op_type, /* PINWEAVER_EAL_{DE|EN}CRYPT */
			     void *res)
{
	EVP_CIPHER_CTX *ctx;
	int rv;
	int len, len_final;

	if (key_size != 128/8) {
		PINWEAVER_EAL_INFO("pinweaver_eal_aes128_cfb: bad key size: %zu", key_size);
		return -1;
	}
	ctx = EVP_CIPHER_CTX_new();
	if (!ctx)
		return -1;
	switch (op_type) {
		case PINWEAVER_EAL_DECRYPT:
			rv = EVP_DecryptInit(ctx, EVP_aes_128_cfb(), key, iv);
			if (rv != 1)
				break;
			rv = EVP_DecryptUpdate(ctx, res, &len, data, size);
			if (rv != 1)
				break;
			rv = EVP_DecryptFinal(ctx, res+len, &len_final);
			break;
		case PINWEAVER_EAL_ENCRYPT:
			rv = EVP_EncryptInit(ctx, EVP_aes_128_cfb(), key, iv);
			if (rv != 1)
				break;
			rv = EVP_EncryptUpdate(ctx, res, &len, data, size);
			if (rv != 1)
				break;
			rv = EVP_EncryptFinal(ctx, res+len, &len_final);
			break;
		default:
			rv = 0;
	}
	EVP_CIPHER_CTX_free(ctx);
	return rv == 1 ? 0 : -1;
}


int pinweaver_eal_safe_memcmp(const void *s1, const void *s2, size_t len)
{
	const uint8_t *us1 = s1;
	const uint8_t *us2 = s2;
	int result = 0;

	while (len--)
		result |= *us1++ ^ *us2++;

	return result != 0;
}

int pinweaver_eal_rand_bytes(void *buf, size_t size)
{
	return RAND_bytes(buf, size) == 1 ? 0 : -1;
}

uint64_t pinweaver_eal_seconds_since_boot(void)
{
	struct sysinfo si;
	if (sysinfo(&si))
		return 0;

	return (uint64_t)si.uptime;
}

int g_device_key_fill[3] = {
	0x01,
	0x00,
	0xFF
};

int pinweaver_eal_get_device_key(int kind,
		void* key /* 256-bit */)
{
	if (kind < 0 || kind >= 3)
		return -1;
	memset(key, g_device_key_fill[kind], 256/8);
	return 0;
}

static void initialize_tpm_key_hash() {
	if (g_tpm_key_hash_volatile_is_set) {
		return;
	}
	g_tpm_key_hash_is_committed = false;
	int fd = open(kTpmKeyHashCommitPath, O_RDONLY);
	if (fd < 0) {
		return;
	}
	read(fd, g_tpm_key_hash_volatile, 32);
	close(fd);
	g_tpm_key_hash_is_committed = true;
	g_tpm_key_hash_volatile_is_set = true;
	PINWEAVER_EAL_INFO("%s: recovered TPM Key hash from %s", __func__,
			kTpmKeyHashCommitPath);
}

int pinweaver_eal_get_tpm_key_hash(uint8_t tpm_key_hash[32], bool* committed)
{
	initialize_tpm_key_hash();
	if (g_tpm_key_hash_volatile_is_set) {
		memcpy(tpm_key_hash, g_tpm_key_hash_volatile, 32);
		*committed = g_tpm_key_hash_is_committed;
		return 0;
	}
	return 1;
}

int pinweaver_eal_set_tpm_key_hash(const uint8_t tpm_key_hash[32])
{
	initialize_tpm_key_hash();
	if (g_tpm_key_hash_is_committed) {
		return 1;
	}
	memcpy(g_tpm_key_hash_volatile, tpm_key_hash, 32);
	g_tpm_key_hash_volatile_is_set = true;
	return 0;
}

int pinweaver_eal_commit_tpm_key_hash()
{
	initialize_tpm_key_hash();
	if (g_tpm_key_hash_is_committed || !g_tpm_key_hash_volatile_is_set) {
		return 1;
	}
	int rc = 0;
	int fd;
	do {
		fd = open(kTpmKeyHashCommitPath, O_WRONLY | O_CREAT, 0644);
		if (fd < 0) {
			rc = fd;
			break;
		}
		if(write(fd, g_tpm_key_hash_volatile, 32) != 32) {
				rc = 1;
		}
		close(fd);
	} while (0);
	if (!rc) {
		g_tpm_key_hash_is_committed = true;
	}
	return rc;
}

uint8_t pinweaver_eal_get_current_pcr_digest(
		const uint8_t bitmask[2],
		uint8_t sha256_of_selected_pcr[32])
{
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

static int tpm_to_openssl_ec_point(const TPMS_ECC_POINT* point,
		const EC_GROUP* ec_group,
		EC_POINT* ec_point)
{
	BN_CTX* ctx = BN_CTX_new();

	BIGNUM* x = BN_CTX_get(ctx);
	BIGNUM* y = BN_CTX_get(ctx);

	if (!x || !y) {
		PINWEAVER_EAL_INFO("%s: failed to create bignums", __func__);
		BN_CTX_free(ctx);
		return -1;
	}

	int res = 1;
	if (res) {
		res = !!BN_bin2bn((const unsigned char*)(point->x.buffer),
				point->x.size, x);
	}
	if (res) {
		res = !!BN_bin2bn((const unsigned char*)(point->y.buffer),
				point->y.size, y);
	}
	if (res) {
		res = EC_POINT_set_affine_coordinates_GFp(ec_group, ec_point,
				x, y, ctx);
	}
	if (!res) {
		PINWEAVER_EAL_INFO("%s: failed to convert: %s",
				__func__, get_openssl_error());
	}
	BN_CTX_free(ctx);
	return res ? -1 : 0;
}

static int bignum_to_tpm_param(const BIGNUM* coord,
		TPM2B_ECC_PARAMETER* param)
{
	const int kEccPointSize = 256/8; /* P-256 curve point size */
	int key_size = BN_num_bytes(coord);
	if (key_size > kEccPointSize) {
		PINWEAVER_EAL_INFO("%s: too big (%d > %d)", __func__, key_size, kEccPointSize);
		return -1;
	}
	memset(param->buffer, 0, sizeof(param->buffer));
	unsigned char* start_pos =
		(unsigned char*)(param->buffer) + kEccPointSize - key_size;
	if (BN_bn2bin(coord, start_pos) != key_size) {
		PINWEAVER_EAL_INFO("%s: unexpected BN_bn2bin size", __func__);
		return -1;
	}

	param->size = kEccPointSize;
	return 0;
}

static int openssl_to_tpm_ec_point(const EC_GROUP* ec_group,
		const EC_POINT* point,
		TPMS_ECC_POINT* tpm_point)
{
	BN_CTX* ctx = BN_CTX_new();

	BIGNUM* x = BN_CTX_get(ctx);
	BIGNUM* y = BN_CTX_get(ctx);

	if (!x || !y) {
		PINWEAVER_EAL_INFO("%s: failed to create bignums", __func__);
		BN_CTX_free(ctx);
		return -1;
	}


	int res = 0;
	if (!res) {
		res = !EC_POINT_get_affine_coordinates_GFp(ec_group, point,
				x, y, ctx);
		if (res) {
			PINWEAVER_EAL_INFO("%s: failed to get X and Y", __func__);
		}
	}
	if (!res) {
		res = bignum_to_tpm_param(x, &tpm_point->x);
	}
	if (!res) {
		res = bignum_to_tpm_param(y, &tpm_point->y);
	}
	BN_CTX_free(ctx);
	return res;
}

static int try_generating_points(const TPMS_ECC_POINT* pub_key,
		TPMS_ECC_POINT* ephemeral_point,
		TPMS_ECC_POINT* z_point)
{
	EC_KEY* ephemeral_key = NULL;
	EC_GROUP* ec_group = NULL;
	EC_POINT* z = NULL;
	EC_POINT* pub = NULL;

	int res = -1;
	do {
		ephemeral_key = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1);
		if (!ephemeral_key) {
			PINWEAVER_EAL_INFO("%s: failed to create new key", __func__);
			break;
		}

		if (!EC_KEY_generate_key(ephemeral_key)) {
			PINWEAVER_EAL_INFO("%s: failed to generate key", __func__);
			break;
		}

		ec_group = EC_GROUP_new_by_curve_name(NID_X9_62_prime256v1);
		if (!ec_group) {
			PINWEAVER_EAL_INFO("%s: failed to create new group", __func__);
			break;
		}

		const EC_POINT* ephemeral_pub =
			EC_KEY_get0_public_key(ephemeral_key);
		const BIGNUM* ephemeral_pri =
			EC_KEY_get0_private_key(ephemeral_key);

		z = EC_POINT_new(ec_group);
		pub = EC_POINT_new(ec_group);
		if (!z || !pub) {
			PINWEAVER_EAL_INFO("%s: failed to create Z and pub points", __func__);
			break;
		}

		if (!tpm_to_openssl_ec_point(pub_key, ec_group, pub)) {
			PINWEAVER_EAL_INFO("%s: failed to convert pub_point", __func__);
			break;
		}

		if (!EC_POINT_mul(ec_group, z, /* n = */ NULL, pub,
				ephemeral_pri, /* ctx = */ NULL)) {
			PINWEAVER_EAL_INFO("%s: failed to compute Z", __func__);
			break;
		}

		if (EC_POINT_is_at_infinity(ec_group, z)) {
			PINWEAVER_EAL_INFO("%s: Z = infinity, retry", __func__);
			break;
		}

		if (openssl_to_tpm_ec_point(ec_group, ephemeral_pub, ephemeral_point)) {
			PINWEAVER_EAL_INFO("%s: failed to convert ephemeral", __func__);
			break;
		}

		if (openssl_to_tpm_ec_point(ec_group, z, z_point)) {
			PINWEAVER_EAL_INFO("%s: failed to convert Z", __func__);
			break;
		}

		res = 0;
	} while(0);

	EC_KEY_free(ephemeral_key);
	EC_GROUP_free(ec_group);
	EC_POINT_free(z);
	EC_POINT_free(pub);

	return res;
}

int pinweaver_eal_generate_ecdh_points(const TPMS_ECC_POINT* pub_key,
		TPMS_ECC_POINT* ephemeral_point,
		TPMS_ECC_POINT* z_point)
{
	const int kRetryLimit = 3;
	int retry;
	for (retry = 0; retry < kRetryLimit; ++retry) {
		if (try_generating_points(pub_key, ephemeral_point,
				z_point) == 0)
			return 0;
	}
	PINWEAVER_EAL_INFO("%s: failed to generate points", __func__);
	return -1;
}
