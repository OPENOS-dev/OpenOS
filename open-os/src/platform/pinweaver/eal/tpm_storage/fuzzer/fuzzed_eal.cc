// Copyright 2021 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>

#include "fuzzer_provider.h"

extern "C" {
#include "pinweaver_eal.h"
#include "pinweaver_eal_tpm.h"
} // extern "C"

extern "C" {

int pinweaver_eal_sha256_init(pinweaver_eal_sha256_ctx_t *ctx) {
  if (ConsumeBoolWithProbability(g_error_response_prob))
    return -1;
  int rv = SHA256_Init(ctx);
  return rv == 1 ? 0 : -1;
}

int pinweaver_eal_sha256_update(pinweaver_eal_sha256_ctx_t *ctx,
                                const void *data, size_t size) {
  if (ConsumeBoolWithProbability(g_error_response_prob))
    return -1;
  int rv = SHA256_Update(ctx, data, size);
  return rv == 1 ? 0 : -1;
}

int pinweaver_eal_sha256_final(pinweaver_eal_sha256_ctx_t *ctx, void *res) {
  int rv = SHA256_Final((unsigned char *)res, ctx);
  if (ConsumeBoolWithProbability(g_error_response_prob))
    return -1;
  return rv == 1 ? 0 : -1;
}

int pinweaver_eal_hmac_sha256_init(pinweaver_eal_hmac_sha256_ctx_t *ctx,
                                   const void *key,
                                   size_t key_size /* in bytes */) {
  if (ConsumeBoolWithProbability(g_error_response_prob))
    return -1;
  *ctx = HMAC_CTX_new();
  if (!*ctx) {
    return -1;
  }
  int rv = HMAC_Init_ex(*ctx, key, key_size, EVP_sha256(), NULL);
  return rv == 1 ? 0 : -1;
}
int pinweaver_eal_hmac_sha256_update(pinweaver_eal_hmac_sha256_ctx_t *ctx,
                                     const void *data, size_t size) {
  if (ConsumeBoolWithProbability(g_error_response_prob))
    return -1;
  int rv = HMAC_Update(*ctx, (const unsigned char *)data, size);
  return rv == 1 ? 0 : -1;
}

int pinweaver_eal_hmac_sha256_final(pinweaver_eal_hmac_sha256_ctx_t *ctx,
                                    void *res) {
  unsigned int len;
  int rv = HMAC_Final(*ctx, (unsigned char *)res, &len);
  HMAC_CTX_free(*ctx);
  *ctx = NULL;
  if (ConsumeBoolWithProbability(g_error_response_prob))
    return -1;
  return rv == 1 ? 0 : -1;
}

int pinweaver_eal_aes256_ctr(const void *key, size_t key_size, /* in bytes */
                             const void *iv, const void *data, size_t size,
                             void *res) {
  if (ConsumeBoolWithProbability(g_error_response_prob))
    return -1;
  // Mocked result
  memcpy(res, data, size);
  return 0;
}

int pinweaver_eal_safe_memcmp(const void *s1, const void *s2, size_t len) {
  if (ConsumeBoolWithProbability(g_error_response_prob)) {
    return g_data_provider->ConsumeIntegralInRange(-1, 1);
  }
  return memcmp(s1, s2, len);
}

int pinweaver_eal_rand_bytes(void *buf, size_t size) {
  if (ConsumeBoolWithProbability(g_error_response_prob))
    return -1;
  FillWithRandomData(buf, size);
  return 0;
}

int pinweaver_eal_get_device_key(int kind, void *key /* 256-bit */) {
  if (ConsumeBoolWithProbability(g_error_response_prob))
    return -1;
  memset(key, kind, 256 / 8);
  return 0;
}

int pinweaver_eal_get_tpm_key_hash(uint8_t tpm_key_hash[32], bool *committed) {
  if (ConsumeBoolWithProbability(g_error_response_prob))
    return -1;
  *committed = g_data_provider->ConsumeBool();

  SHA256_CTX ctx;
  if (SHA256_Init(&ctx) == 1) {
    SHA256_Update(&ctx, kSaltingX.buffer, kSaltingX.size);
    SHA256_Update(&ctx, kSaltingY.buffer, kSaltingY.size);
    SHA256_Final(tpm_key_hash, &ctx);
  }

  return 0;
}

int pinweaver_eal_generate_ecdh_points(const TPMS_ECC_POINT *pub_key,
                                       TPMS_ECC_POINT *ephemeral_point,
                                       TPMS_ECC_POINT *z_point) {
  if (ConsumeBoolWithProbability(g_error_response_prob))
    return -1;
  ephemeral_point->x = Consume2B<TPM2B_ECC_PARAMETER>();
  ephemeral_point->y = Consume2B<TPM2B_ECC_PARAMETER>();
  z_point->x = Consume2B<TPM2B_ECC_PARAMETER>();
  z_point->y = Consume2B<TPM2B_ECC_PARAMETER>();
  return 0;
}

} // extern "C"