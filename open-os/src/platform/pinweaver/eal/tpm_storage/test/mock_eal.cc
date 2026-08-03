// Copyright 2021 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>

#include "mock_eal.h"

using testing::_;
using testing::Invoke;
using testing::NiceMock;
using testing::Return;

std::unique_ptr<EalInterface> g_Eal;
EalInterface *g_EalPtr = nullptr;

EalInterface *GetEal() {
  if (!g_EalPtr) {
    if (!g_Eal) {
      g_Eal.reset(new NiceMock<MockEalInterface>());
    }
    g_EalPtr = g_Eal.get();
  }
  return g_EalPtr;
}
void SetEal(EalInterface *interface) { g_EalPtr = interface; }

MockEalInterface::MockEalInterface() {
  ON_CALL(*this, sha256_init(_)).WillByDefault(Return(0));
  ON_CALL(*this, sha256_update(_, _, _)).WillByDefault(Return(0));
  ON_CALL(*this, sha256_final(_, _))
      .WillByDefault(
          Invoke([](pinweaver_eal_sha256_ctx_t * /* ctx */, void *res) {
            memset(res, kHashByte, 256 / 8);
            return 0;
          }));

  ON_CALL(*this, hmac_sha256_init(_, _, _)).WillByDefault(Return(0));
  ON_CALL(*this, hmac_sha256_update(_, _, _)).WillByDefault(Return(0));
  ON_CALL(*this, hmac_sha256_final(_, _))
      .WillByDefault(
          Invoke([](pinweaver_eal_hmac_sha256_ctx_t * /* ctx */, void *res) {
            memset(res, kHashByte, 256 / 8);
            return 0;
          }));

  ON_CALL(*this, aes256_ctr(_, _, _, _, _, _))
      .WillByDefault(
          Invoke([](const void *key, size_t key_size, /* in bytes */
                    const void *iv, const void *data, size_t size, void *res) {
            // Mock encryption: do nothing
            memcpy(res, data, size);
            return 0;
          }));

  ON_CALL(*this, safe_memcmp(_, _, _))
      .WillByDefault(Invoke([](const void *s1, const void *s2, size_t len) {
        return memcmp(s1, s2, len);
      }));
  ON_CALL(*this, rand_bytes(_, _))
      .WillByDefault(Invoke([](void *buf, size_t size) {
        memset(buf, kRandByte, size);
        return 0;
      }));
  ON_CALL(*this, get_device_key(_, _))
      .WillByDefault(Invoke([this](int kind, void *key /* 256-bit */) {
        memset(key, kind + this->past_generation_, 256 / 8);
        return 0;
      }));
  ON_CALL(*this, get_tpm_key_hash(_, _))
      .WillByDefault(Invoke([this](uint8_t tpm_key_hash[32], bool *committed) {
        *committed = this->tpm_key_committed_ ? 1 : 0;
        if (!this->tpm_key_set_)
          return 1;
        memset(tpm_key_hash, kHashByte, 32);
        return 0;
      }));
  ON_CALL(*this, generate_ecdh_points(_, _, _)).WillByDefault(Return(0));
}

extern "C" {

int pinweaver_eal_sha256_init(pinweaver_eal_sha256_ctx_t *ctx) {
  return GetEal()->sha256_init(ctx);
}

int pinweaver_eal_sha256_update(pinweaver_eal_sha256_ctx_t *ctx,
                                const void *data, size_t size) {
  return GetEal()->sha256_update(ctx, data, size);
}

int pinweaver_eal_sha256_final(pinweaver_eal_sha256_ctx_t *ctx, void *res) {
  return GetEal()->sha256_final(ctx, res);
}

int pinweaver_eal_hmac_sha256_init(pinweaver_eal_hmac_sha256_ctx_t *ctx,
                                   const void *key,
                                   size_t key_size /* in bytes */) {
  return GetEal()->hmac_sha256_init(ctx, key, key_size);
}

int pinweaver_eal_hmac_sha256_update(pinweaver_eal_hmac_sha256_ctx_t *ctx,
                                     const void *data, size_t size) {
  return GetEal()->hmac_sha256_update(ctx, data, size);
}

int pinweaver_eal_hmac_sha256_final(pinweaver_eal_hmac_sha256_ctx_t *ctx,
                                    void *res) {
  return GetEal()->hmac_sha256_final(ctx, res);
}

int pinweaver_eal_aes256_ctr(const void *key, size_t key_size, /* in bytes */
                             const void *iv, const void *data, size_t size,
                             void *res) {
  return GetEal()->aes256_ctr(key, key_size, iv, data, size, res);
}

int pinweaver_eal_safe_memcmp(const void *s1, const void *s2, size_t len) {
  return GetEal()->safe_memcmp(s1, s2, len);
}

int pinweaver_eal_rand_bytes(void *buf, size_t size) {
  return GetEal()->rand_bytes(buf, size);
}

int pinweaver_eal_get_device_key(int kind, void *key /* 256-bit */) {
  return GetEal()->get_device_key(kind, key);
}

int pinweaver_eal_get_tpm_key_hash(uint8_t tpm_key_hash[32], bool *committed) {
  return GetEal()->get_tpm_key_hash(tpm_key_hash, committed);
}

int pinweaver_eal_generate_ecdh_points(const TPMS_ECC_POINT *pub_key,
                                       TPMS_ECC_POINT *ephemeral_point,
                                       TPMS_ECC_POINT *z_point) {
  return GetEal()->generate_ecdh_points(pub_key, ephemeral_point, z_point);
}

} // extern "C"