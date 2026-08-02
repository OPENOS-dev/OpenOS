// Copyright 2021 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string.h>

#include "hmac_authorization_delegate.h"
#include "pinweaver_eal.h"
#include "pinweaver_eal_tpm.h"

#define NONCE_MIN_SIZE 16
#define NONCE_MAX_SIZE 32
#define DECRYPT_SESSION (1 << 5)
#define ENCRYPT_SESSION (1 << 6)
#define LABEL_SIZE 4
#define AES_KEY_SIZE (128/8)
#define AES_IV_SIZE AES_KEY_SIZE
#define MAX_DECRYPT_SIZE 256
#define CONTINUE_SESSION 1
#define MAX_DIGEST_SIZE (sizeof(TPMU_HA))
#define SESSION_KEY_SIZE (256/8)

typedef struct HMAC_KEY {
  size_t size;
  uint8_t buffer[2 * MAX_DIGEST_SIZE];
} HMAC_KEY;

typedef struct HMAC_PROCESSOR {
  pinweaver_eal_hmac_sha256_ctx_t hmac;
  int res;
  int init_res;
} HMAC_PROCESSOR;

static void HmacInit(
  HMAC_PROCESSOR* p,
  const HMAC_KEY* key
) {
  p->init_res = p->res = pinweaver_eal_hmac_sha256_init(&p->hmac, key->buffer, key->size);  
}

static void HmacUpdate(
  HMAC_PROCESSOR* p,
  const void* data,
  size_t size
) {
  if (!p->res) {
    p->res = pinweaver_eal_hmac_sha256_update(&p->hmac, data, size);
  }
}

static int HmacFinal(
  HMAC_PROCESSOR* p,
  void* result
) {
  if (p->init_res) {
    return p->init_res;
  }
  p->res |= pinweaver_eal_hmac_sha256_final(&p->hmac, result);
  return p->res;
}

static void HmacKeyInit(HMAC_KEY* key) {
  key->size = 0;
}

static int HmacKeyAppend(HMAC_KEY* key, const TPM2B_DIGEST* data) {
  if (key->size + data->size > sizeof(key->buffer)) {
    return -1;
  }
  if (pinweaver_eal_memcpy_s(key->buffer + key->size,
      sizeof(key->buffer) - key->size,
      data->buffer, data->size)) {
    return -1;
  }
  key->size += data->size;
  return 0;
}

// This method implements the key derivation function used in the TPM.
// NOTE: It only returns 32 byte keys.
static int CreateKey(
    TSS_AUTHORIZATION_DELEGATE_HMAC* delegate,
    const HMAC_KEY* hmac_key,
    const TSS_SRC_DATA_BUF* label,
    const TPM2B_NONCE* nonce_newer,
    const TPM2B_NONCE* nonce_older,
    uint8_t* key /* size=SHA256_DIGEST_SIZE */) {
  uint32_t counter_buf;
  TSS_DST_DATA_BUF counter = {sizeof(uint32_t), 0, (uint8_t *)&counter_buf};
  uint32_t digest_size_bits_buf;
  TSS_DST_DATA_BUF digest_size_bits = {sizeof(uint32_t), 0,
                                     (uint8_t *)&digest_size_bits_buf};
  const uint32_t kOne = 1;
  const uint32_t kDigestBits = 256;

  if (tss_Serialize_uint32_t(&kOne, &counter) != TPM_RC_SUCCESS ||
      tss_Serialize_uint32_t(&kDigestBits, &digest_size_bits) != TPM_RC_SUCCESS) {
    return -1;
  }

  HMAC_PROCESSOR hp;
  HmacInit(&hp, hmac_key);
  HmacUpdate(&hp, counter.buffer, counter.size);
  HmacUpdate(&hp, label->buffer, label->size);
  HmacUpdate(&hp, nonce_newer->buffer, nonce_newer->size);
  HmacUpdate(&hp, nonce_older->buffer, nonce_older->size);
  HmacUpdate(&hp, digest_size_bits.buffer, digest_size_bits.size);
  return HmacFinal(&hp, key);
}

// This method performs an AES operation using a 128 bit key.
// |operation_type| can be either AES_ENCRYPT or AES_DECRYPT and it
// determines if the operation is an encryption or decryption.
static int AesOperation(
    TSS_AUTHORIZATION_DELEGATE_HMAC* delegate,
    TSS_SRC_DATA_BUF* parameter,
    const TPM2B_NONCE* nonce_newer,
    const TPM2B_NONCE* nonce_older,
    int operation_type) {
  if (parameter->size > MAX_DECRYPT_SIZE) {
    return -1;
  }
  if (delegate->session_key_.size > sizeof(delegate->session_key_.buffer)) {
    return -1;
  }
  if (delegate->entity_authorization_value_.size >
      sizeof(delegate->entity_authorization_value_.buffer)) {
    return -1;
  }
  const TSS_SRC_DATA_BUF kLabelCFB = {LABEL_SIZE, (uint8_t *)"CFB"};
  HMAC_KEY hmac_key;
  HmacKeyInit(&hmac_key);
  if (HmacKeyAppend(&hmac_key, &delegate->session_key_)) {
    return -1;
  }
  if (HmacKeyAppend(&hmac_key, &delegate->entity_authorization_value_)) {
    return -1;
  }
  uint8_t compound_key[SESSION_KEY_SIZE];
  if (CreateKey(delegate,
                &hmac_key,
                &kLabelCFB,
                nonce_newer,
                nonce_older,
                compound_key)) {
    return -1;
  }
  unsigned char aes_key[AES_KEY_SIZE];
  unsigned char aes_iv[AES_IV_SIZE];
  if (pinweaver_eal_memcpy_s(aes_key, AES_KEY_SIZE, &compound_key[0],
      AES_KEY_SIZE)) {
    return -1;
  }
  if (pinweaver_eal_memcpy_s(aes_iv, AES_IV_SIZE, &compound_key[AES_KEY_SIZE],
      AES_IV_SIZE)) {
    return -1;
  }
  unsigned char decrypted[MAX_DECRYPT_SIZE];
  if (pinweaver_eal_aes128_cfb(aes_key, AES_KEY_SIZE, aes_iv,
                               parameter->buffer, parameter->size,
                               operation_type, decrypted) != 0) {
    return -1;
  }
  if (pinweaver_eal_memcpy_s(parameter->buffer, parameter->size,
      decrypted, parameter->size)) {
    return -1;
  }
  return 0;
}

// This method regenerates the caller nonce. The new nonce is the same
// length as the previous nonce. The buffer is filled with random data.
// NOTE: This operation is DESTRUCTIVE, and rewrites the caller_nonce_ field.
static void RegenerateCallerNonce(TSS_AUTHORIZATION_DELEGATE_HMAC* delegate) {
  // nonce_size is guaranteed to be less than 32 bytes and greater than 16.
  pinweaver_eal_rand_bytes(delegate->caller_nonce_.buffer,
                           delegate->caller_nonce_.size);
}

int tss_GetCommandAuthorizationHmac(
    TSS_AUTHORIZATION_DELEGATE_HMAC* delegate,
    const TSS_SRC_DATA_BUF* command_hash,
    bool is_command_parameter_encryption_possible,
    bool is_response_parameter_encryption_possible,
    TSS_DST_DATA_BUF* authorization) {
  if (!delegate->session_handle_) {
    authorization->size = 0;
    return -1;
  }
  if (delegate->caller_nonce_.size > sizeof(delegate->caller_nonce_.buffer)) {
    authorization->size = 0;
    return -1;
  }
  TPMS_AUTH_COMMAND auth;
  auth.session_handle = delegate->session_handle_;
  if (!delegate->nonce_generated_) {
    RegenerateCallerNonce(delegate);
  }
  auth.nonce = delegate->caller_nonce_;
  auth.session_attributes = CONTINUE_SESSION;
  if (delegate->is_parameter_encryption_enabled_) {
    if (is_command_parameter_encryption_possible) {
      auth.session_attributes |= DECRYPT_SESSION;
    }
    if (is_response_parameter_encryption_possible) {
      auth.session_attributes |= ENCRYPT_SESSION;
    }
  }
  // We reset the |nonce_generated| flag in preparation of the next command.
  delegate->nonce_generated_ = false;
  UINT8 attributes_bytes_buf;
  TSS_DST_DATA_BUF attributes_bytes = {sizeof(attributes_bytes_buf), 0,
                                     &attributes_bytes_buf};
  if (tss_Serialize_TPMA_SESSION(&auth.session_attributes, &attributes_bytes) !=
      TPM_RC_SUCCESS) {
    return -1;
  }

  if (delegate->session_key_.size > sizeof(delegate->session_key_.buffer)) {
    authorization->size = 0;
    return -1;
  }
  if (delegate->entity_authorization_value_.size >
      sizeof(delegate->entity_authorization_value_.buffer)) {
    authorization->size = 0;
    return -1;
  }

  HMAC_KEY hmac_key;
  HmacKeyInit(&hmac_key);
  if (HmacKeyAppend(&hmac_key, &delegate->session_key_)) {
    return -1;
  }
  if (!delegate->use_entity_authorization_for_encryption_only_) {
    if (HmacKeyAppend(&hmac_key, &delegate->entity_authorization_value_)) {
      return -1;
    }
  }

  if (delegate->tpm_nonce_.size > sizeof(delegate->tpm_nonce_.buffer)) {
    authorization->size = 0;
    return -1;
  }

  HMAC_PROCESSOR hp;
  HmacInit(&hp, &hmac_key);
  HmacUpdate(&hp, command_hash->buffer, command_hash->size);
  HmacUpdate(&hp, delegate->caller_nonce_.buffer, delegate->caller_nonce_.size);
  HmacUpdate(&hp, delegate->tpm_nonce_.buffer, delegate->tpm_nonce_.size);
  HmacUpdate(&hp, attributes_bytes.buffer, attributes_bytes.size);
  if (HmacFinal(&hp, auth.hmac.buffer))
    return -1;
  auth.hmac.size = SHA256_DIGEST_SIZE;

  TPM_RC serialize_error = tss_Serialize_TPMS_AUTH_COMMAND(&auth, authorization);
  if (serialize_error != TPM_RC_SUCCESS) {
    return -1;
  }
  return 0;
}

int tss_CheckResponseAuthorizationHmac(
    TSS_AUTHORIZATION_DELEGATE_HMAC* delegate,
    const TSS_SRC_DATA_BUF* response_hash,
    const TSS_SRC_DATA_BUF* authorization) {
  if (!delegate->session_handle_) {
    return -1;
  }
  TPMS_AUTH_RESPONSE auth_response;
  TSS_SRC_DATA_BUF mutable_auth_string = {authorization->size,
                                        authorization->buffer};
  TPM_RC parse_error;
  parse_error = tss_Parse_TPMS_AUTH_RESPONSE(&mutable_auth_string, &auth_response,
                                             NULL);
  if (parse_error != TPM_RC_SUCCESS) {
    return -1;
  }
  if (mutable_auth_string.size != 0) {
    return -1;
  }
  if (auth_response.hmac.size != SHA256_DIGEST_SIZE) {
    return -1;
  }
  if (auth_response.nonce.size < NONCE_MIN_SIZE ||
      auth_response.nonce.size > NONCE_MAX_SIZE) {
    return -1;
  }
  delegate->tpm_nonce_ = auth_response.nonce;
  UINT8 attributes_bytes_buf;
  TSS_DST_DATA_BUF attributes_bytes = {sizeof(attributes_bytes_buf), 0,
                                     &attributes_bytes_buf};
  if (tss_Serialize_TPMA_SESSION(&auth_response.session_attributes,
                                 &attributes_bytes) != TPM_RC_SUCCESS) {
    return -1;
  }

  if (delegate->session_key_.size > sizeof(delegate->session_key_.buffer)) {
    return -1;
  }

  HMAC_KEY hmac_key;
  HmacKeyInit(&hmac_key);
  if (HmacKeyAppend(&hmac_key, &delegate->session_key_)) {
    return -1;
  }
  if (!delegate->use_entity_authorization_for_encryption_only_) {
    // In a special case with TPM2_HierarchyChangeAuth, we need to use the
    // auth_value that was set.
    if (delegate->future_authorization_value_set_) {
      if (HmacKeyAppend(&hmac_key, &delegate->future_authorization_value_)) {
        return -1;
      }
      delegate->future_authorization_value_set_ = false;
    } else {
      if (HmacKeyAppend(&hmac_key, &delegate->entity_authorization_value_)) {
        return -1;
      }
    }
  }

  unsigned char digest[SHA256_DIGEST_SIZE];
  HMAC_PROCESSOR hp;
  HmacInit(&hp, &hmac_key);
  HmacUpdate(&hp, response_hash->buffer, response_hash->size);
  HmacUpdate(&hp, delegate->tpm_nonce_.buffer, delegate->tpm_nonce_.size);
  HmacUpdate(&hp, delegate->caller_nonce_.buffer, delegate->caller_nonce_.size);
  HmacUpdate(&hp, attributes_bytes.buffer, attributes_bytes.size);
  if (HmacFinal(&hp, digest))
    return -1;

  if (pinweaver_eal_safe_memcmp(digest, auth_response.hmac.buffer,
                                SHA256_DIGEST_SIZE)) {
    return -1;
  }
  return 0;
}

int tss_EncryptCommandParameterHmac(
    TSS_AUTHORIZATION_DELEGATE_HMAC* delegate,
    TSS_SRC_DATA_BUF* parameter) {
  if (!delegate->session_handle_) {
    return -1;
  }
  if (!delegate->is_parameter_encryption_enabled_) {
    // No parameter encryption enabled.
    return 0;
  }
  if (delegate->caller_nonce_.size > sizeof(delegate->caller_nonce_.buffer)) {
    return -1;
  }
  if (delegate->tpm_nonce_.size > sizeof(delegate->tpm_nonce_.buffer)) {
    return -1;
  }
  RegenerateCallerNonce(delegate);
  delegate->nonce_generated_ = true;
  return AesOperation(delegate, parameter,
                      &delegate->caller_nonce_, &delegate->tpm_nonce_,
                      PINWEAVER_EAL_ENCRYPT);
}

int tss_DecryptResponseParameterHmac(
    TSS_AUTHORIZATION_DELEGATE_HMAC* delegate,
    TSS_SRC_DATA_BUF* parameter) {
  if (!delegate->session_handle_) {
    return -1;
  }
  if (!delegate->is_parameter_encryption_enabled_) {
    // No parameter decryption enabled.
    return 0;
  }
  return AesOperation(delegate, parameter,
                      &delegate->tpm_nonce_, &delegate->caller_nonce_,
                      PINWEAVER_EAL_DECRYPT);
}

int tss_GetTpmNonceHmac(
    TSS_AUTHORIZATION_DELEGATE_HMAC* delegate,
    TSS_DST_DATA_BUF* nonce) {
  if (!delegate->tpm_nonce_.size)
    return -1;
  if (nonce->max_size < delegate->tpm_nonce_.size)
    return -1;
  if (pinweaver_eal_memcpy_s(nonce->buffer, nonce->max_size,
      &delegate->tpm_nonce_.buffer, delegate->tpm_nonce_.size)) {
    return -1;
  }
  nonce->size = delegate->tpm_nonce_.size;
  return 0;
}

int tss_InitSessionHmac(
    TSS_AUTHORIZATION_DELEGATE_HMAC* delegate,
    TPM_HANDLE session_handle,
    const TPM2B_NONCE* tpm_nonce,
    const TPM2B_NONCE* caller_nonce,
    const TPM2B_DIGEST* salt,
    const TPM2B_DIGEST* bind_auth_value,
    bool enable_parameter_encryption) {
  memset((void *)delegate, 0, sizeof(TSS_AUTHORIZATION_DELEGATE_HMAC));
  delegate->session_handle_ = session_handle;
  if (caller_nonce->size < NONCE_MIN_SIZE
      || caller_nonce->size > NONCE_MAX_SIZE
      || tpm_nonce->size < NONCE_MIN_SIZE
      || tpm_nonce->size > NONCE_MAX_SIZE) {
    return -1;
  }
  if (pinweaver_eal_memcpy_s(&delegate->tpm_nonce_, sizeof(TPM2B_NONCE),
      tpm_nonce, sizeof(TPM2B_NONCE))) {
    return -1;
  }
  if (pinweaver_eal_memcpy_s(&delegate->caller_nonce_, sizeof(TPM2B_NONCE),
      caller_nonce, sizeof(TPM2B_NONCE))) {
    return -1;
  }
  delegate->is_parameter_encryption_enabled_ = enable_parameter_encryption;
  if (salt->size == 0 && (!bind_auth_value || bind_auth_value->size == 0)) {
    // SessionKey is set to the empty string for unsalted and
    // unbound sessions.
    delegate->session_key_.size = 0;
  } else {
    TSS_SRC_DATA_BUF kLabelATH = {LABEL_SIZE, (uint8_t *)"ATH"};
    HMAC_KEY hmac_key;
    HmacKeyInit(&hmac_key);
    if (bind_auth_value) {
      if (HmacKeyAppend(&hmac_key, bind_auth_value)) {
        return -1;
      }
    }
    if (HmacKeyAppend(&hmac_key, salt)) {
      return -1;
    }
    if (CreateKey(delegate, &hmac_key, &kLabelATH,
                  &delegate->tpm_nonce_, &delegate->caller_nonce_,
                  delegate->session_key_.buffer)) {
      return -1;
    }
    delegate->session_key_.size = SESSION_KEY_SIZE;
  }
  return 0;
}
