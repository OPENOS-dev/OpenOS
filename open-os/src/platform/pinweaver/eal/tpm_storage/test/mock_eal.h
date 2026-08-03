// Copyright 2021 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MOCK_EAL_H_
#define MOCK_EAL_H_

extern "C" {
#include "pinweaver_eal.h"
#include "pinweaver_eal_tpm.h"
} // extern "C"

#include <gmock/gmock.h>

class EalInterface {
public:
  EalInterface() = default;
  virtual ~EalInterface() = default;
  virtual int sha256_init(pinweaver_eal_sha256_ctx_t *ctx) = 0;
  virtual int sha256_update(pinweaver_eal_sha256_ctx_t *ctx, const void *data,
                            size_t size) = 0;
  virtual int sha256_final(pinweaver_eal_sha256_ctx_t *ctx, void *res) = 0;
  virtual int hmac_sha256_init(pinweaver_eal_hmac_sha256_ctx_t *ctx,
                               const void *key,
                               size_t key_size /* in bytes */) = 0;
  virtual int hmac_sha256_update(pinweaver_eal_hmac_sha256_ctx_t *ctx,
                                 const void *data, size_t size) = 0;
  virtual int hmac_sha256_final(pinweaver_eal_hmac_sha256_ctx_t *ctx,
                                void *res) = 0;
  virtual int aes256_ctr(const void *key, size_t key_size, /* in bytes */
                         const void *iv, const void *data, size_t size,
                         void *res) = 0;
  virtual int safe_memcmp(const void *s1, const void *s2, size_t len) = 0;
  virtual int rand_bytes(void *buf, size_t size) = 0;
  virtual int get_device_key(int kind, void *key /* 256-bit */) = 0;
  virtual int get_tpm_key_hash(uint8_t tpm_key_hash[32], bool *committed) = 0;
  virtual int generate_ecdh_points(const TPMS_ECC_POINT *pub_key,
                                   TPMS_ECC_POINT *ephemeral_point,
                                   TPMS_ECC_POINT *z_point) = 0;
};

class MockEalInterface : public EalInterface {
public:
  static const int kHashByte = 0x55;
  static const int kRandByte = 0x77;

  MockEalInterface();
  ~MockEalInterface() override = default;
  MOCK_METHOD1(sha256_init, int(pinweaver_eal_sha256_ctx_t *ctx));
  MOCK_METHOD3(sha256_update, int(pinweaver_eal_sha256_ctx_t *ctx,
                                  const void *data, size_t size));
  MOCK_METHOD2(sha256_final, int(pinweaver_eal_sha256_ctx_t *ctx, void *res));
  MOCK_METHOD3(hmac_sha256_init,
               int(pinweaver_eal_hmac_sha256_ctx_t *ctx, const void *key,
                   size_t key_size /* in bytes */));
  MOCK_METHOD3(hmac_sha256_update, int(pinweaver_eal_hmac_sha256_ctx_t *ctx,
                                       const void *data, size_t size));
  MOCK_METHOD2(hmac_sha256_final,
               int(pinweaver_eal_hmac_sha256_ctx_t *ctx, void *res));
  MOCK_METHOD6(aes256_ctr,
               int(const void *key, size_t key_size, /* in bytes */
                   const void *iv, const void *data, size_t size, void *res));
  MOCK_METHOD3(safe_memcmp, int(const void *s1, const void *s2, size_t len));
  MOCK_METHOD2(rand_bytes, int(void *buf, size_t size));
  MOCK_METHOD2(get_device_key, int(int kind, void *key /* 256-bit */));
  MOCK_METHOD2(get_tpm_key_hash,
               int(uint8_t tpm_key_hash[32], bool *committed));
  MOCK_METHOD3(generate_ecdh_points,
               int(const TPMS_ECC_POINT *pub_key,
                   TPMS_ECC_POINT *ephemeral_point, TPMS_ECC_POINT *z_point));

  void set_past_generation(int generation) {
    this->past_generation_ = generation;
  }
  void set_tpm_key_committed(bool value) { this->tpm_key_committed_ = value; }
  void set_tpm_key_set(bool value) { this->tpm_key_set_ = value; }

private:
  int past_generation_ = 0;
  bool tpm_key_committed_ = true;
  bool tpm_key_set_ = true;
};

EalInterface *GetEal();
void SetEal(EalInterface *interface);

#endif // MOCK_EAL_H_
