// Copyright 2021 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <map>
#include <memory>

#include "fuzzer_provider.h"

extern "C" {
#include <tpm_storage_internal.h>
#include <tss.h>
} // extern "C"

// Data stored in NV spaces
std::map<TPM_HANDLE, std::string> nv_spaces;

bool ConsumeBoolWithProbability(uint32_t probability) {
  return g_data_provider->ConsumeIntegralInRange<uint32_t>(0, 99) < probability;
}

uint32_t ConsumeErrorResponseCode() {
  // Decide between realistic and purely random value.
  if (ConsumeBoolWithProbability(g_pure_random_prob)) {
    return g_data_provider->ConsumeIntegralInRange<TPM_RC>(1, UINT32_MAX);
  }
  return g_data_provider->PickValueInArray({
      TPM_RC_NV_UNINITIALIZED,
      TPM_RC_NV_DEFINED,
      TPM_RC_BAD_AUTH + TPM_RC_S + TPM_RC_1,
      TPM_RC_AUTH_FAIL + TPM_RC_S + TPM_RC_1,
      TPM_RC_HANDLE + TPM_RC_H + TPM_RC_1,
  });
}

void FillWithRandomData(void *buffer, size_t size) {
  memset(buffer, 0xFF, size);
  std::string random_data = g_data_provider->ConsumeBytesAsString(size);
  memcpy(buffer, random_data.data(), random_data.size());
}

void Fill2BInternal(uint16_t *size, unsigned char *buffer, uint16_t max_size) {
  FillWithRandomData(buffer, max_size);

  if (ConsumeBoolWithProbability(g_pure_random_prob)) {
    *size = g_data_provider->ConsumeIntegral<uint16_t>();
  } else {
    if (ConsumeBoolWithProbability(g_error_response_prob)) {
      *size = g_data_provider->ConsumeIntegralInRange<uint16_t>(0, max_size);
    } else {
      *size = max_size;
    }
  }
}

TPM2B_NAME ConsumeName() {
  TPM2B_NAME tpm2b;
  Fill2BInternal(&tpm2b.size, tpm2b.name, sizeof(tpm2b.name));
  return tpm2b;
}

TPM2B_PUBLIC ConsumePublic() {
  TPM2B_PUBLIC pub;
  FillWithRandomData(&pub.public_area, sizeof(pub.public_area));
  if (ConsumeBoolWithProbability(g_pure_random_prob)) {
    pub.size = g_data_provider->ConsumeIntegral<uint16_t>();
  } else {
    pub.size = sizeof(pub.public_area);
    pub.public_area.type = TPM_ALG_ECC;
    pub.public_area.parameters.ecc_detail.curve_id = TPM_ECC_NIST_P256;
      pub.public_area.object_attributes = REQUIRED_SALTING_KEY_ATTR;
    pub.public_area.unique.ecc = kPubSaltingKey;
    if (ConsumeBoolWithProbability(g_error_response_prob)) {
      if (g_data_provider->ConsumeBool()) {
        pub.public_area.type =
          g_data_provider->ConsumeIntegral<TPMI_ALG_PUBLIC>();
        pub.public_area.parameters.ecc_detail.curve_id =
          g_data_provider->ConsumeIntegral<TPMI_ECC_CURVE>();
        pub.public_area.object_attributes =
          g_data_provider->ConsumeIntegral<TPMA_OBJECT>();
      } else {
        pub.public_area.unique.ecc.x = Consume2B<TPM2B_ECC_PARAMETER>();
        pub.public_area.unique.ecc.y = Consume2B<TPM2B_ECC_PARAMETER>();
      }
    }
  }
  return pub;
}

uint16_t ExpectedNVDataSize(TPM_HANDLE handle) {
  if (handle == TREE_DESCRIPTOR_HANDLE) {
    return sizeof(tpm_storage_tree_descriptor_t);
  } else {
    return sizeof(tpm_storage_log_entry_t);
  }
}

void SetNVPublicArea(TPM_HANDLE handle, TPMS_NV_PUBLIC *public_area) {
  // Policy digest for nvmem spaces:
  // - Initial state
  //   0000000000000000000000000000000000000000000000000000000000000000
  // - PolicyAuthValue:
  //     <new> = sha256(<previous> || 0000016B)
  //   8fcd2169ab92694e0c633f1ab772842b8241bbc20288981fc7ac1eddc1fddb0e
  // - PolicyCommandCode(NV_ChangeAuth):
  //     <new> = sha256(<previous> || 0000016C || 0000013B)
  //   363ac945b6457c47c31f3355dba0db27de8db213d6250c6bf79685003f9fe7ab
  const uint8_t kNvPolicy[SHA256_DIGEST_SIZE] = {
      0x36, 0x3a, 0xc9, 0x45, 0xb6, 0x45, 0x7c, 0x47, 0xc3, 0x1f, 0x33,
      0x55, 0xdb, 0xa0, 0xdb, 0x27, 0xde, 0x8d, 0xb2, 0x13, 0xd6, 0x25,
      0x0c, 0x6b, 0xf7, 0x96, 0x85, 0x00, 0x3f, 0x9f, 0xe7, 0xab,
  };
  memset(public_area, 0, sizeof(TPMS_NV_PUBLIC));
  public_area->nv_index = handle;
  public_area->name_alg = TPM_ALG_SHA256;
  public_area->attributes =
      TPMA_NV_AUTHREAD | TPMA_NV_AUTHWRITE | TPMA_NV_NO_DA;
  if (ConsumeBoolWithProbability(g_written_nv_prob))
    public_area->attributes |= TPMA_NV_WRITTEN;
  memcpy(public_area->auth_policy.buffer, kNvPolicy, SHA256_DIGEST_SIZE);
  public_area->auth_policy.size = SHA256_DIGEST_SIZE;
  public_area->data_size = ExpectedNVDataSize(handle);
}

TPM2B_NV_PUBLIC ConsumeNVPublic(TPM_HANDLE handle) {
  TPM2B_NV_PUBLIC pub;
  FillWithRandomData(&pub.nv_public, sizeof(pub.nv_public));
  if (ConsumeBoolWithProbability(g_pure_random_prob)) {
    pub.size = g_data_provider->ConsumeIntegral<uint16_t>();
  } else {
    pub.size = sizeof(pub.nv_public);
    if (!ConsumeBoolWithProbability(g_error_response_prob)) {
      SetNVPublicArea(handle, &pub.nv_public);
    }
  }
  return pub;
}

TPM2B_NAME CalcSha256Name(const void *data, size_t size) {
  /* Serialized TPM_ALG_SHA256 */
  TPM2B_NAME name;
  const uint8_t kSerializedAlgSha256[2] = {0x00, 0x0B};

  memcpy(name.name, kSerializedAlgSha256, sizeof(kSerializedAlgSha256));
  name.size = sizeof(kSerializedAlgSha256) + SHA256_DIGEST_SIZE;
  uint8_t *hash = name.name + sizeof(kSerializedAlgSha256);
  memset(hash, 0, SHA256_DIGEST_SIZE);

  SHA256_CTX ctx;
  if (SHA256_Init(&ctx) == 1) {
    SHA256_Update(&ctx, data, size);
    SHA256_Final(hash, &ctx);
  }
  return name;
}

TPM2B_NAME CalcNVName(const TPMS_NV_PUBLIC *public_area) {
  return CalcSha256Name(public_area, sizeof(TPMS_NV_PUBLIC));
}

std::string CreateTreeDescrNV() {
  tpm_storage_tree_descriptor_t tree;
  memset(&tree, 0, sizeof(tree));
  tree.version = 0;
  tree.restart_count = 1;
  return std::string((char *)&tree, sizeof(tree));
}

std::string CreateLogEntryNV() {
  tpm_storage_log_entry_t entry;
  entry.version = 0;
  entry.counter = g_data_provider->ConsumeIntegral<uint32_t>();
  memset(entry.iv, 0xFF, sizeof(entry.iv));

  log_sensitive_data_t plaintext;
  memset(&plaintext, 0xA5, sizeof(plaintext));
  SHA256_CTX ctx;
  if (SHA256_Init(&ctx) == 1) {
    SHA256_Update(&ctx, &plaintext.entry, sizeof(plaintext.entry));
    SHA256_Final(plaintext.hash, &ctx);
  }
  // Assumes mocked aes256_ctr implementation
  memcpy(entry.encrypted_data, &plaintext, sizeof(log_sensitive_data_t));
  return std::string((char *)&entry, sizeof(entry));
}

std::string CreateNV(TPM_HANDLE handle) {
  const size_t size = ExpectedNVDataSize(handle);
  if (ConsumeBoolWithProbability(g_pure_random_prob)) {
    return g_data_provider->ConsumeRandomLengthString(2 * size);
  }
  if (handle == TREE_DESCRIPTOR_HANDLE) {
    return CreateTreeDescrNV();
  }
  return CreateLogEntryNV();
}

std::string GetNV(TPM_HANDLE handle) {
  auto elem = nv_spaces.find(handle);
  if (elem != nv_spaces.end()) {
    return elem->second;
  }
  std::string data = CreateNV(handle);
  nv_spaces[handle] = data;
  return data;
}

void SetNV(TPM_HANDLE handle, std::string data) { nv_spaces[handle] = data; }

extern "C" {

TPM_RC tss_StartAuthSession(
    const TPMI_DH_OBJECT tpm_key, const TPM2B_NAME *tpm_key_name,
    const TPMI_DH_ENTITY bind, const TPM2B_NAME *bind_name,
    const TPM2B_NONCE *nonce_caller,
    const TPM2B_ENCRYPTED_SECRET *encrypted_salt, const TPM_SE session_type,
    const TPMT_SYM_DEF *symmetric, const TPMI_ALG_HASH auth_hash,
    TPMI_SH_AUTH_SESSION *session_handle, TPM2B_NONCE *nonce_tpm,
    TSS_AUTHORIZATION_DELEGATE *authorization_delegate) {
  if (ConsumeBoolWithProbability(g_error_response_prob)) {
    return ConsumeErrorResponseCode();
  }
  *session_handle = g_data_provider->ConsumeIntegral<TPMI_SH_AUTH_SESSION>();
  *nonce_tpm = Consume2B<TPM2B_NONCE>();
  return TPM_RC_SUCCESS;
}

TPM_RC tss_ReadPublic(const TPMI_DH_OBJECT object_handle,
                      const TPM2B_NAME *object_handle_name,
                      TPM2B_PUBLIC *out_public, TPM2B_NAME *name,
                      TPM2B_NAME *qualified_name,
                      TSS_AUTHORIZATION_DELEGATE *authorization_delegate) {
  if (ConsumeBoolWithProbability(g_error_response_prob)) {
    return ConsumeErrorResponseCode();
  }
  *out_public = ConsumePublic();
  *name = ConsumeName();
  *qualified_name = ConsumeName();
  return TPM_RC_SUCCESS;
}

TPM_RC
tss_PolicyCommandCode(const TPMI_SH_POLICY policy_session,
                      const TPM2B_NAME *policy_session_name, const TPM_CC code,
                      TSS_AUTHORIZATION_DELEGATE *authorization_delegate) {
  if (ConsumeBoolWithProbability(g_error_response_prob)) {
    return ConsumeErrorResponseCode();
  }
  return TPM_RC_SUCCESS;
}

TPM_RC tss_PolicyAuthValue(const TPMI_SH_POLICY policy_session,
                           const TPM2B_NAME *policy_session_name,
                           TSS_AUTHORIZATION_DELEGATE *authorization_delegate) {
  if (ConsumeBoolWithProbability(g_error_response_prob)) {
    return ConsumeErrorResponseCode();
  }
  return TPM_RC_SUCCESS;
}

TPM_RC tss_FlushContext(const TPMI_DH_CONTEXT flush_handle,
                        TSS_AUTHORIZATION_DELEGATE *authorization_delegate) {
  if (ConsumeBoolWithProbability(g_error_response_prob)) {
    return ConsumeErrorResponseCode();
  }
  return TPM_RC_SUCCESS;
}

TPM_RC tss_NV_DefineSpace(const TPMI_RH_PROVISION auth_handle,
                          const TPM2B_NAME *auth_handle_name,
                          const TPM2B_AUTH *auth,
                          const TPM2B_NV_PUBLIC *public_info,
                          TSS_AUTHORIZATION_DELEGATE *authorization_delegate) {
  if (ConsumeBoolWithProbability(g_error_response_prob)) {
    return ConsumeErrorResponseCode();
  }
  return TPM_RC_SUCCESS;
}

TPM_RC tss_NV_ReadPublic(const TPMI_RH_NV_INDEX nv_index,
                         const TPM2B_NAME *nv_index_name,
                         TPM2B_NV_PUBLIC *nv_public, TPM2B_NAME *nv_name,
                         TSS_AUTHORIZATION_DELEGATE *authorization_delegate) {
  if (ConsumeBoolWithProbability(g_error_response_prob)) {
    return ConsumeErrorResponseCode();
  }
  *nv_public = ConsumeNVPublic(nv_index);
  *nv_name = CalcNVName(&nv_public->nv_public);
  return TPM_RC_SUCCESS;
}

TPM_RC tss_NV_Write(const TPMI_RH_NV_AUTH auth_handle,
                    const TPM2B_NAME *auth_handle_name,
                    const TPMI_RH_NV_INDEX nv_index,
                    const TPM2B_NAME *nv_index_name,
                    const TPM2B_MAX_NV_BUFFER *data, const UINT16 offset,
                    TSS_AUTHORIZATION_DELEGATE *authorization_delegate) {
  if (ConsumeBoolWithProbability(g_error_response_prob)) {
    return ConsumeErrorResponseCode();
  }
  // Knowing the implementation offset must be 0
  if (offset != 0)
    abort();
  size_t size = std::min((size_t)data->size, sizeof(data->buffer));
  SetNV(nv_index, std::string((char *)data->buffer, size));
  return TPM_RC_SUCCESS;
}

TPM_RC tss_NV_Read(const TPMI_RH_NV_AUTH auth_handle,
                   const TPM2B_NAME *auth_handle_name,
                   const TPMI_RH_NV_INDEX nv_index,
                   const TPM2B_NAME *nv_index_name, const UINT16 size,
                   const UINT16 offset, TPM2B_MAX_NV_BUFFER *data,
                   TSS_AUTHORIZATION_DELEGATE *authorization_delegate) {
  if (ConsumeBoolWithProbability(g_error_response_prob)) {
    return ConsumeErrorResponseCode();
  }
  // Knowing the implementation offset must be 0
  if (offset != 0)
    abort();
  std::string data_str = GetNV(nv_index);
  if (data_str.size() > sizeof(data->buffer))
    data_str.resize(sizeof(data->buffer));
  memcpy(data->buffer, data_str.data(), data_str.size());
  data->size = data_str.size();
  return TPM_RC_SUCCESS;
}

TPM_RC tss_NV_ChangeAuth(const TPMI_RH_NV_INDEX nv_index,
                         const TPM2B_NAME *nv_index_name,
                         const TPM2B_AUTH *new_auth,
                         TSS_AUTHORIZATION_DELEGATE *authorization_delegate) {
  if (ConsumeBoolWithProbability(g_error_response_prob)) {
    return ConsumeErrorResponseCode();
  }
  return TPM_RC_SUCCESS;
}

int tss_InitSessionHmac(TSS_AUTHORIZATION_DELEGATE_HMAC *delegate,
                        TPM_HANDLE session_handle, const TPM2B_NONCE *tpm_nonce,
                        const TPM2B_NONCE *caller_nonce,
                        const TPM2B_DIGEST *salt,
                        const TPM2B_DIGEST *bind_auth_value,
                        bool enable_parameter_encryption) {
  if (ConsumeBoolWithProbability(g_error_response_prob)) {
    return -1;
  }
  return 0;
}

TPM_RC tss_Serialize_TPMS_NV_PUBLIC(const TPMS_NV_PUBLIC *value,
                                    TSS_DST_DATA_BUF *buffer) {
  if (ConsumeBoolWithProbability(g_error_response_prob)) {
    return ConsumeErrorResponseCode();
  }
  // Mock serialization
  if (buffer->max_size < sizeof(TPMS_NV_PUBLIC))
    return TPM_RC_INSUFFICIENT;
  memcpy(buffer->buffer, value, sizeof(TPMS_NV_PUBLIC));
  buffer->size = sizeof(TPMS_NV_PUBLIC);
  return TPM_RC_SUCCESS;
}

TPM_RC tss_Serialize_TPMS_ECC_POINT(const TPMS_ECC_POINT *value,
                                    TSS_DST_DATA_BUF *buffer) {
  if (ConsumeBoolWithProbability(g_error_response_prob)) {
    return ConsumeErrorResponseCode();
  }
  // Mock serialization
  if (buffer->max_size < sizeof(TPMS_ECC_POINT))
    return TPM_RC_INSUFFICIENT;
  memcpy(buffer->buffer, value, sizeof(TPMS_ECC_POINT));
  buffer->size = sizeof(TPMS_ECC_POINT);
  return TPM_RC_SUCCESS;
}

} // extern "C"
