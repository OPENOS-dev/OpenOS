// Copyright 2021 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_TRUNKS_TSS_SERDE_H_
#define MINI_TRUNKS_TSS_SERDE_H_

#include "tss_types.h"

typedef struct TSS_SRC_DATA_BUF {
  size_t size;
  uint8_t* buffer;
} TSS_SRC_DATA_BUF;

typedef struct TSS_DST_DATA_BUF {
  size_t max_size;
  size_t size;
  uint8_t* buffer;
} TSS_DST_DATA_BUF;

size_t tss_GetNumberOfRequestHandles(TPM_CC command_code);
size_t tss_GetNumberOfResponseHandles(TPM_CC command_code);

TPM_RC tss_Serialize_uint8_t(const uint8_t* value,
                         TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_uint8_t(TSS_SRC_DATA_BUF* buffer,
                     uint8_t* value,
                     TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_int8_t(const int8_t* value, TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_int8_t(TSS_SRC_DATA_BUF* buffer,
                    int8_t* value,
                    TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_int(const int* value, TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_int(TSS_SRC_DATA_BUF* buffer,
                 int* value,
                 TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_uint16_t(const uint16_t* value,
                          TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_uint16_t(TSS_SRC_DATA_BUF* buffer,
                      uint16_t* value,
                      TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_int16_t(const int16_t* value,
                         TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_int16_t(TSS_SRC_DATA_BUF* buffer,
                     int16_t* value,
                     TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_uint32_t(const uint32_t* value,
                          TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_uint32_t(TSS_SRC_DATA_BUF* buffer,
                      uint32_t* value,
                      TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_int32_t(const int32_t* value,
                         TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_int32_t(TSS_SRC_DATA_BUF* buffer,
                     int32_t* value,
                     TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_uint64_t(const uint64_t* value,
                          TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_uint64_t(TSS_SRC_DATA_BUF* buffer,
                      uint64_t* value,
                      TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_int64_t(const int64_t* value,
                         TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_int64_t(TSS_SRC_DATA_BUF* buffer,
                     int64_t* value,
                     TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_UINT8(const UINT8* value, TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_UINT8(TSS_SRC_DATA_BUF* buffer,
                   UINT8* value,
                   TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_BYTE(const BYTE* value, TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_BYTE(TSS_SRC_DATA_BUF* buffer,
                  BYTE* value,
                  TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_INT8(const INT8* value, TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_INT8(TSS_SRC_DATA_BUF* buffer,
                  INT8* value,
                  TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_BOOL(const BOOL* value, TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_BOOL(TSS_SRC_DATA_BUF* buffer,
                  BOOL* value,
                  TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_UINT16(const UINT16* value, TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_UINT16(TSS_SRC_DATA_BUF* buffer,
                    UINT16* value,
                    TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_INT16(const INT16* value, TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_INT16(TSS_SRC_DATA_BUF* buffer,
                   INT16* value,
                   TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_UINT32(const UINT32* value, TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_UINT32(TSS_SRC_DATA_BUF* buffer,
                    UINT32* value,
                    TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_INT32(const INT32* value, TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_INT32(TSS_SRC_DATA_BUF* buffer,
                   INT32* value,
                   TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_UINT64(const UINT64* value, TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_UINT64(TSS_SRC_DATA_BUF* buffer,
                    UINT64* value,
                    TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_INT64(const INT64* value, TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_INT64(TSS_SRC_DATA_BUF* buffer,
                   INT64* value,
                   TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPM_ALGORITHM_ID(const TPM_ALGORITHM_ID* value,
                                  TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPM_ALGORITHM_ID(TSS_SRC_DATA_BUF* buffer,
                              TPM_ALGORITHM_ID* value,
                              TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPM_MODIFIER_INDICATOR(
    const TPM_MODIFIER_INDICATOR* value, TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPM_MODIFIER_INDICATOR(TSS_SRC_DATA_BUF* buffer,
                                    TPM_MODIFIER_INDICATOR* value,
                                    TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPM_AUTHORIZATION_SIZE(
    const TPM_AUTHORIZATION_SIZE* value, TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPM_AUTHORIZATION_SIZE(TSS_SRC_DATA_BUF* buffer,
                                    TPM_AUTHORIZATION_SIZE* value,
                                    TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPM_PARAMETER_SIZE(
    const TPM_PARAMETER_SIZE* value, TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPM_PARAMETER_SIZE(TSS_SRC_DATA_BUF* buffer,
                                TPM_PARAMETER_SIZE* value,
                                TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPM_KEY_SIZE(const TPM_KEY_SIZE* value,
                              TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPM_KEY_SIZE(TSS_SRC_DATA_BUF* buffer,
                          TPM_KEY_SIZE* value,
                          TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPM_KEY_BITS(const TPM_KEY_BITS* value,
                              TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPM_KEY_BITS(TSS_SRC_DATA_BUF* buffer,
                          TPM_KEY_BITS* value,
                          TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPM_HANDLE(const TPM_HANDLE* value,
                            TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPM_HANDLE(TSS_SRC_DATA_BUF* buffer,
                        TPM_HANDLE* value,
                        TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPM2B_NONCE(const TPM2B_NONCE* value,
                             TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPM2B_NONCE(TSS_SRC_DATA_BUF* buffer,
                         TPM2B_NONCE* value,
                         TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPM2B_AUTH(const TPM2B_AUTH* value,
                            TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPM2B_AUTH(TSS_SRC_DATA_BUF* buffer,
                        TPM2B_AUTH* value,
                        TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPM2B_OPERAND(const TPM2B_OPERAND* value,
                               TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPM2B_OPERAND(TSS_SRC_DATA_BUF* buffer,
                           TPM2B_OPERAND* value,
                           TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMS_SCHEME_HMAC(const TPMS_SCHEME_HMAC* value,
                                  TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMS_SCHEME_HMAC(TSS_SRC_DATA_BUF* buffer,
                              TPMS_SCHEME_HMAC* value,
                              TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMS_SCHEME_RSASSA(
    const TPMS_SCHEME_RSASSA* value, TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMS_SCHEME_RSASSA(TSS_SRC_DATA_BUF* buffer,
                                TPMS_SCHEME_RSASSA* value,
                                TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMS_SCHEME_RSAPSS(
    const TPMS_SCHEME_RSAPSS* value, TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMS_SCHEME_RSAPSS(TSS_SRC_DATA_BUF* buffer,
                                TPMS_SCHEME_RSAPSS* value,
                                TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMS_SCHEME_ECDSA(const TPMS_SCHEME_ECDSA* value,
                                   TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMS_SCHEME_ECDSA(TSS_SRC_DATA_BUF* buffer,
                               TPMS_SCHEME_ECDSA* value,
                               TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMS_SCHEME_SM2(const TPMS_SCHEME_SM2* value,
                                 TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMS_SCHEME_SM2(TSS_SRC_DATA_BUF* buffer,
                             TPMS_SCHEME_SM2* value,
                             TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMS_SCHEME_ECSCHNORR(
    const TPMS_SCHEME_ECSCHNORR* value, TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMS_SCHEME_ECSCHNORR(TSS_SRC_DATA_BUF* buffer,
                                   TPMS_SCHEME_ECSCHNORR* value,
                                   TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMI_YES_NO(const TPMI_YES_NO* value,
                             TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMI_YES_NO(TSS_SRC_DATA_BUF* buffer,
                         TPMI_YES_NO* value,
                         TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMI_DH_OBJECT(const TPMI_DH_OBJECT* value,
                                TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMI_DH_OBJECT(TSS_SRC_DATA_BUF* buffer,
                            TPMI_DH_OBJECT* value,
                            TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMI_DH_PERSISTENT(
    const TPMI_DH_PERSISTENT* value, TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMI_DH_PERSISTENT(TSS_SRC_DATA_BUF* buffer,
                                TPMI_DH_PERSISTENT* value,
                                TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMI_DH_ENTITY(const TPMI_DH_ENTITY* value,
                                TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMI_DH_ENTITY(TSS_SRC_DATA_BUF* buffer,
                            TPMI_DH_ENTITY* value,
                            TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMI_DH_PCR(const TPMI_DH_PCR* value,
                             TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMI_DH_PCR(TSS_SRC_DATA_BUF* buffer,
                         TPMI_DH_PCR* value,
                         TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMI_SH_AUTH_SESSION(
    const TPMI_SH_AUTH_SESSION* value, TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMI_SH_AUTH_SESSION(TSS_SRC_DATA_BUF* buffer,
                                  TPMI_SH_AUTH_SESSION* value,
                                  TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMI_SH_HMAC(const TPMI_SH_HMAC* value,
                              TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMI_SH_HMAC(TSS_SRC_DATA_BUF* buffer,
                          TPMI_SH_HMAC* value,
                          TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMI_SH_POLICY(const TPMI_SH_POLICY* value,
                                TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMI_SH_POLICY(TSS_SRC_DATA_BUF* buffer,
                            TPMI_SH_POLICY* value,
                            TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMI_DH_CONTEXT(const TPMI_DH_CONTEXT* value,
                                 TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMI_DH_CONTEXT(TSS_SRC_DATA_BUF* buffer,
                             TPMI_DH_CONTEXT* value,
                             TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMI_RH_HIERARCHY(const TPMI_RH_HIERARCHY* value,
                                   TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMI_RH_HIERARCHY(TSS_SRC_DATA_BUF* buffer,
                               TPMI_RH_HIERARCHY* value,
                               TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMI_RH_ENABLES(const TPMI_RH_ENABLES* value,
                                 TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMI_RH_ENABLES(TSS_SRC_DATA_BUF* buffer,
                             TPMI_RH_ENABLES* value,
                             TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMI_RH_HIERARCHY_AUTH(
    const TPMI_RH_HIERARCHY_AUTH* value, TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMI_RH_HIERARCHY_AUTH(TSS_SRC_DATA_BUF* buffer,
                                    TPMI_RH_HIERARCHY_AUTH* value,
                                    TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMI_RH_PLATFORM(const TPMI_RH_PLATFORM* value,
                                  TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMI_RH_PLATFORM(TSS_SRC_DATA_BUF* buffer,
                              TPMI_RH_PLATFORM* value,
                              TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMI_RH_OWNER(const TPMI_RH_OWNER* value,
                               TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMI_RH_OWNER(TSS_SRC_DATA_BUF* buffer,
                           TPMI_RH_OWNER* value,
                           TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMI_RH_ENDORSEMENT(
    const TPMI_RH_ENDORSEMENT* value, TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMI_RH_ENDORSEMENT(TSS_SRC_DATA_BUF* buffer,
                                 TPMI_RH_ENDORSEMENT* value,
                                 TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMI_RH_PROVISION(const TPMI_RH_PROVISION* value,
                                   TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMI_RH_PROVISION(TSS_SRC_DATA_BUF* buffer,
                               TPMI_RH_PROVISION* value,
                               TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMI_RH_CLEAR(const TPMI_RH_CLEAR* value,
                               TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMI_RH_CLEAR(TSS_SRC_DATA_BUF* buffer,
                           TPMI_RH_CLEAR* value,
                           TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMI_RH_NV_AUTH(const TPMI_RH_NV_AUTH* value,
                                 TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMI_RH_NV_AUTH(TSS_SRC_DATA_BUF* buffer,
                             TPMI_RH_NV_AUTH* value,
                             TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMI_RH_LOCKOUT(const TPMI_RH_LOCKOUT* value,
                                 TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMI_RH_LOCKOUT(TSS_SRC_DATA_BUF* buffer,
                             TPMI_RH_LOCKOUT* value,
                             TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMI_RH_NV_INDEX(const TPMI_RH_NV_INDEX* value,
                                  TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMI_RH_NV_INDEX(TSS_SRC_DATA_BUF* buffer,
                              TPMI_RH_NV_INDEX* value,
                              TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMI_ALG_HASH(const TPMI_ALG_HASH* value,
                               TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMI_ALG_HASH(TSS_SRC_DATA_BUF* buffer,
                           TPMI_ALG_HASH* value,
                           TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMI_ALG_ASYM(const TPMI_ALG_ASYM* value,
                               TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMI_ALG_ASYM(TSS_SRC_DATA_BUF* buffer,
                           TPMI_ALG_ASYM* value,
                           TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMI_ALG_SYM(const TPMI_ALG_SYM* value,
                              TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMI_ALG_SYM(TSS_SRC_DATA_BUF* buffer,
                          TPMI_ALG_SYM* value,
                          TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMI_ALG_SYM_OBJECT(
    const TPMI_ALG_SYM_OBJECT* value, TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMI_ALG_SYM_OBJECT(TSS_SRC_DATA_BUF* buffer,
                                 TPMI_ALG_SYM_OBJECT* value,
                                 TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMI_ALG_SYM_MODE(const TPMI_ALG_SYM_MODE* value,
                                   TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMI_ALG_SYM_MODE(TSS_SRC_DATA_BUF* buffer,
                               TPMI_ALG_SYM_MODE* value,
                               TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMI_ALG_KDF(const TPMI_ALG_KDF* value,
                              TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMI_ALG_KDF(TSS_SRC_DATA_BUF* buffer,
                          TPMI_ALG_KDF* value,
                          TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMI_ALG_SIG_SCHEME(
    const TPMI_ALG_SIG_SCHEME* value, TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMI_ALG_SIG_SCHEME(TSS_SRC_DATA_BUF* buffer,
                                 TPMI_ALG_SIG_SCHEME* value,
                                 TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMI_ECC_KEY_EXCHANGE(
    const TPMI_ECC_KEY_EXCHANGE* value, TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMI_ECC_KEY_EXCHANGE(TSS_SRC_DATA_BUF* buffer,
                                   TPMI_ECC_KEY_EXCHANGE* value,
                                   TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMI_ST_COMMAND_TAG(
    const TPMI_ST_COMMAND_TAG* value, TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMI_ST_COMMAND_TAG(TSS_SRC_DATA_BUF* buffer,
                                 TPMI_ST_COMMAND_TAG* value,
                                 TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMI_ST_ATTEST(const TPMI_ST_ATTEST* value,
                                TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMI_ST_ATTEST(TSS_SRC_DATA_BUF* buffer,
                            TPMI_ST_ATTEST* value,
                            TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMI_AES_KEY_BITS(const TPMI_AES_KEY_BITS* value,
                                   TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMI_AES_KEY_BITS(TSS_SRC_DATA_BUF* buffer,
                               TPMI_AES_KEY_BITS* value,
                               TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMI_SM4_KEY_BITS(const TPMI_SM4_KEY_BITS* value,
                                   TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMI_SM4_KEY_BITS(TSS_SRC_DATA_BUF* buffer,
                               TPMI_SM4_KEY_BITS* value,
                               TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMI_ALG_KEYEDHASH_SCHEME(
    const TPMI_ALG_KEYEDHASH_SCHEME* value, TSS_DST_DATA_BUF* buffer);

TPM_RC
Parse_TPMI_ALG_KEYEDHASH_SCHEME(TSS_SRC_DATA_BUF* buffer,
                                TPMI_ALG_KEYEDHASH_SCHEME* value,
                                TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMI_ALG_ASYM_SCHEME(
    const TPMI_ALG_ASYM_SCHEME* value, TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMI_ALG_ASYM_SCHEME(TSS_SRC_DATA_BUF* buffer,
                                  TPMI_ALG_ASYM_SCHEME* value,
                                  TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMI_ALG_RSA_SCHEME(
    const TPMI_ALG_RSA_SCHEME* value, TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMI_ALG_RSA_SCHEME(TSS_SRC_DATA_BUF* buffer,
                                 TPMI_ALG_RSA_SCHEME* value,
                                 TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMI_ALG_RSA_DECRYPT(
    const TPMI_ALG_RSA_DECRYPT* value, TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMI_ALG_RSA_DECRYPT(TSS_SRC_DATA_BUF* buffer,
                                  TPMI_ALG_RSA_DECRYPT* value,
                                  TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMI_RSA_KEY_BITS(const TPMI_RSA_KEY_BITS* value,
                                   TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMI_RSA_KEY_BITS(TSS_SRC_DATA_BUF* buffer,
                               TPMI_RSA_KEY_BITS* value,
                               TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMI_ALG_ECC_SCHEME(
    const TPMI_ALG_ECC_SCHEME* value, TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMI_ALG_ECC_SCHEME(TSS_SRC_DATA_BUF* buffer,
                                 TPMI_ALG_ECC_SCHEME* value,
                                 TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMI_ECC_CURVE(const TPMI_ECC_CURVE* value,
                                TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMI_ECC_CURVE(TSS_SRC_DATA_BUF* buffer,
                            TPMI_ECC_CURVE* value,
                            TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMI_ALG_PUBLIC(const TPMI_ALG_PUBLIC* value,
                                 TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMI_ALG_PUBLIC(TSS_SRC_DATA_BUF* buffer,
                             TPMI_ALG_PUBLIC* value,
                             TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMA_ALGORITHM(const TPMA_ALGORITHM* value,
                                TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMA_ALGORITHM(TSS_SRC_DATA_BUF* buffer,
                            TPMA_ALGORITHM* value,
                            TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMA_OBJECT(const TPMA_OBJECT* value,
                             TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMA_OBJECT(TSS_SRC_DATA_BUF* buffer,
                         TPMA_OBJECT* value,
                         TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMA_SESSION(const TPMA_SESSION* value,
                              TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMA_SESSION(TSS_SRC_DATA_BUF* buffer,
                          TPMA_SESSION* value,
                          TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMA_LOCALITY(const TPMA_LOCALITY* value,
                               TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMA_LOCALITY(TSS_SRC_DATA_BUF* buffer,
                           TPMA_LOCALITY* value,
                           TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMA_PERMANENT(const TPMA_PERMANENT* value,
                                TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMA_PERMANENT(TSS_SRC_DATA_BUF* buffer,
                            TPMA_PERMANENT* value,
                            TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMA_STARTUP_CLEAR(
    const TPMA_STARTUP_CLEAR* value, TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMA_STARTUP_CLEAR(TSS_SRC_DATA_BUF* buffer,
                                TPMA_STARTUP_CLEAR* value,
                                TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMA_MEMORY(const TPMA_MEMORY* value,
                             TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMA_MEMORY(TSS_SRC_DATA_BUF* buffer,
                         TPMA_MEMORY* value,
                         TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMA_CC(const TPMA_CC* value,
                         TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMA_CC(TSS_SRC_DATA_BUF* buffer,
                                   TPMA_CC* value,
                                   TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPM_NV_INDEX(const TPM_NV_INDEX* value,
                              TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPM_NV_INDEX(TSS_SRC_DATA_BUF* buffer,
                          TPM_NV_INDEX* value,
                          TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMA_NV(const TPMA_NV* value,
                         TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMA_NV(TSS_SRC_DATA_BUF* buffer,
                     TPMA_NV* value,
                     TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPM_SPEC(const TPM_SPEC* value,
                          TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPM_SPEC(TSS_SRC_DATA_BUF* buffer,
                      TPM_SPEC* value,
                      TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPM_GENERATED(const TPM_GENERATED* value,
                               TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPM_GENERATED(TSS_SRC_DATA_BUF* buffer,
                           TPM_GENERATED* value,
                           TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPM_ALG_ID(const TPM_ALG_ID* value,
                            TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPM_ALG_ID(TSS_SRC_DATA_BUF* buffer,
                        TPM_ALG_ID* value,
                        TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPM_ECC_CURVE(const TPM_ECC_CURVE* value,
                               TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPM_ECC_CURVE(TSS_SRC_DATA_BUF* buffer,
                           TPM_ECC_CURVE* value,
                           TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPM_CC(const TPM_CC* value, TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPM_CC(TSS_SRC_DATA_BUF* buffer,
                    TPM_CC* value,
                    TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPM_RC(const TPM_RC* value, TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPM_RC(TSS_SRC_DATA_BUF* buffer,
                    TPM_RC* value,
                    TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPM_CLOCK_ADJUST(const TPM_CLOCK_ADJUST* value,
                                  TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPM_CLOCK_ADJUST(TSS_SRC_DATA_BUF* buffer,
                              TPM_CLOCK_ADJUST* value,
                              TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPM_EO(const TPM_EO* value, TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPM_EO(TSS_SRC_DATA_BUF* buffer,
                                  TPM_EO* value,
                                  TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPM_ST(const TPM_ST* value, TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPM_ST(TSS_SRC_DATA_BUF* buffer,
                                  TPM_ST* value,
                                  TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPM_SU(const TPM_SU* value, TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPM_SU(TSS_SRC_DATA_BUF* buffer,
                                  TPM_SU* value,
                                  TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPM_SE(const TPM_SE* value, TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPM_SE(TSS_SRC_DATA_BUF* buffer,
                                  TPM_SE* value,
                                  TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPM_CAP(const TPM_CAP* value,
                         TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPM_CAP(TSS_SRC_DATA_BUF* buffer,
                     TPM_CAP* value,
                     TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPM_PT(const TPM_PT* value, TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPM_PT(TSS_SRC_DATA_BUF* buffer,
                                  TPM_PT* value,
                                  TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPM_PT_PCR(const TPM_PT_PCR* value,
                            TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPM_PT_PCR(TSS_SRC_DATA_BUF* buffer,
                        TPM_PT_PCR* value,
                        TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPM_PS(const TPM_PS* value, TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPM_PS(TSS_SRC_DATA_BUF* buffer,
                                  TPM_PS* value,
                                  TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPM_HT(const TPM_HT* value, TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPM_HT(TSS_SRC_DATA_BUF* buffer,
                                  TPM_HT* value,
                                  TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPM_RH(const TPM_RH* value, TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPM_RH(TSS_SRC_DATA_BUF* buffer,
                                  TPM_RH* value,
                                  TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPM_HC(const TPM_HC* value, TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPM_HC(TSS_SRC_DATA_BUF* buffer,
                                  TPM_HC* value,
                                  TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMS_ALGORITHM_DESCRIPTION(
    const TPMS_ALGORITHM_DESCRIPTION* value, TSS_DST_DATA_BUF* buffer);

TPM_RC
Parse_TPMS_ALGORITHM_DESCRIPTION(TSS_SRC_DATA_BUF* buffer,
                                 TPMS_ALGORITHM_DESCRIPTION* value,
                                 TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMT_HA(const TPMT_HA* value,
                         TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMT_HA(TSS_SRC_DATA_BUF* buffer,
                     TPMT_HA* value,
                     TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPM2B_DIGEST(const TPM2B_DIGEST* value,
                              TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPM2B_DIGEST(TSS_SRC_DATA_BUF* buffer,
                          TPM2B_DIGEST* value,
                          TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPM2B_DATA(const TPM2B_DATA* value,
                            TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPM2B_DATA(TSS_SRC_DATA_BUF* buffer,
                        TPM2B_DATA* value,
                        TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPM2B_EVENT(const TPM2B_EVENT* value,
                             TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPM2B_EVENT(TSS_SRC_DATA_BUF* buffer,
                         TPM2B_EVENT* value,
                         TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPM2B_MAX_BUFFER(const TPM2B_MAX_BUFFER* value,
                                  TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPM2B_MAX_BUFFER(TSS_SRC_DATA_BUF* buffer,
                              TPM2B_MAX_BUFFER* value,
                              TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPM2B_MAX_NV_BUFFER(
    const TPM2B_MAX_NV_BUFFER* value, TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPM2B_MAX_NV_BUFFER(TSS_SRC_DATA_BUF* buffer,
                                 TPM2B_MAX_NV_BUFFER* value,
                                 TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPM2B_TIMEOUT(const TPM2B_TIMEOUT* value,
                               TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPM2B_TIMEOUT(TSS_SRC_DATA_BUF* buffer,
                           TPM2B_TIMEOUT* value,
                           TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPM2B_IV(const TPM2B_IV* value,
                          TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPM2B_IV(TSS_SRC_DATA_BUF* buffer,
                      TPM2B_IV* value,
                      TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPM2B_NAME(const TPM2B_NAME* value,
                            TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPM2B_NAME(TSS_SRC_DATA_BUF* buffer,
                        TPM2B_NAME* value,
                        TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMS_PCR_SELECT(const TPMS_PCR_SELECT* value,
                                 TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMS_PCR_SELECT(TSS_SRC_DATA_BUF* buffer,
                             TPMS_PCR_SELECT* value,
                             TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMS_PCR_SELECTION(
    const TPMS_PCR_SELECTION* value, TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMS_PCR_SELECTION(TSS_SRC_DATA_BUF* buffer,
                                TPMS_PCR_SELECTION* value,
                                TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMT_TK_CREATION(const TPMT_TK_CREATION* value,
                                  TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMT_TK_CREATION(TSS_SRC_DATA_BUF* buffer,
                              TPMT_TK_CREATION* value,
                              TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMT_TK_VERIFIED(const TPMT_TK_VERIFIED* value,
                                  TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMT_TK_VERIFIED(TSS_SRC_DATA_BUF* buffer,
                              TPMT_TK_VERIFIED* value,
                              TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMT_TK_AUTH(const TPMT_TK_AUTH* value,
                              TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMT_TK_AUTH(TSS_SRC_DATA_BUF* buffer,
                          TPMT_TK_AUTH* value,
                          TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMT_TK_HASHCHECK(const TPMT_TK_HASHCHECK* value,
                                   TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMT_TK_HASHCHECK(TSS_SRC_DATA_BUF* buffer,
                               TPMT_TK_HASHCHECK* value,
                               TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMS_ALG_PROPERTY(const TPMS_ALG_PROPERTY* value,
                                   TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMS_ALG_PROPERTY(TSS_SRC_DATA_BUF* buffer,
                               TPMS_ALG_PROPERTY* value,
                               TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMS_TAGGED_PROPERTY(
    const TPMS_TAGGED_PROPERTY* value, TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMS_TAGGED_PROPERTY(TSS_SRC_DATA_BUF* buffer,
                                  TPMS_TAGGED_PROPERTY* value,
                                  TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMS_TAGGED_PCR_SELECT(
    const TPMS_TAGGED_PCR_SELECT* value, TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMS_TAGGED_PCR_SELECT(TSS_SRC_DATA_BUF* buffer,
                                    TPMS_TAGGED_PCR_SELECT* value,
                                    TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPML_CC(const TPML_CC* value,
                         TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPML_CC(TSS_SRC_DATA_BUF* buffer,
                     TPML_CC* value,
                     TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPML_CCA(const TPML_CCA* value,
                          TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPML_CCA(TSS_SRC_DATA_BUF* buffer,
                      TPML_CCA* value,
                      TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPML_ALG(const TPML_ALG* value,
                          TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPML_ALG(TSS_SRC_DATA_BUF* buffer,
                      TPML_ALG* value,
                      TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPML_HANDLE(const TPML_HANDLE* value,
                             TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPML_HANDLE(TSS_SRC_DATA_BUF* buffer,
                         TPML_HANDLE* value,
                         TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPML_DIGEST(const TPML_DIGEST* value,
                             TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPML_DIGEST(TSS_SRC_DATA_BUF* buffer,
                         TPML_DIGEST* value,
                         TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPML_DIGEST_VALUES(
    const TPML_DIGEST_VALUES* value, TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPML_DIGEST_VALUES(TSS_SRC_DATA_BUF* buffer,
                                TPML_DIGEST_VALUES* value,
                                TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPM2B_DIGEST_VALUES(
    const TPM2B_DIGEST_VALUES* value, TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPM2B_DIGEST_VALUES(TSS_SRC_DATA_BUF* buffer,
                                 TPM2B_DIGEST_VALUES* value,
                                 TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPML_PCR_SELECTION(
    const TPML_PCR_SELECTION* value, TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPML_PCR_SELECTION(TSS_SRC_DATA_BUF* buffer,
                                TPML_PCR_SELECTION* value,
                                TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPML_ALG_PROPERTY(const TPML_ALG_PROPERTY* value,
                                   TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPML_ALG_PROPERTY(TSS_SRC_DATA_BUF* buffer,
                               TPML_ALG_PROPERTY* value,
                               TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPML_TAGGED_TPM_PROPERTY(
    const TPML_TAGGED_TPM_PROPERTY* value, TSS_DST_DATA_BUF* buffer);

TPM_RC
Parse_TPML_TAGGED_TPM_PROPERTY(TSS_SRC_DATA_BUF* buffer,
                               TPML_TAGGED_TPM_PROPERTY* value,
                               TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPML_TAGGED_PCR_PROPERTY(
    const TPML_TAGGED_PCR_PROPERTY* value, TSS_DST_DATA_BUF* buffer);

TPM_RC
Parse_TPML_TAGGED_PCR_PROPERTY(TSS_SRC_DATA_BUF* buffer,
                               TPML_TAGGED_PCR_PROPERTY* value,
                               TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPML_ECC_CURVE(const TPML_ECC_CURVE* value,
                                TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPML_ECC_CURVE(TSS_SRC_DATA_BUF* buffer,
                            TPML_ECC_CURVE* value,
                            TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMS_CAPABILITY_DATA(
    const TPMS_CAPABILITY_DATA* value, TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMS_CAPABILITY_DATA(TSS_SRC_DATA_BUF* buffer,
                                  TPMS_CAPABILITY_DATA* value,
                                  TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMS_CLOCK_INFO(const TPMS_CLOCK_INFO* value,
                                 TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMS_CLOCK_INFO(TSS_SRC_DATA_BUF* buffer,
                             TPMS_CLOCK_INFO* value,
                             TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMS_TIME_INFO(const TPMS_TIME_INFO* value,
                                TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMS_TIME_INFO(TSS_SRC_DATA_BUF* buffer,
                            TPMS_TIME_INFO* value,
                            TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMS_TIME_ATTEST_INFO(
    const TPMS_TIME_ATTEST_INFO* value, TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMS_TIME_ATTEST_INFO(TSS_SRC_DATA_BUF* buffer,
                                   TPMS_TIME_ATTEST_INFO* value,
                                   TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMS_CERTIFY_INFO(const TPMS_CERTIFY_INFO* value,
                                   TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMS_CERTIFY_INFO(TSS_SRC_DATA_BUF* buffer,
                               TPMS_CERTIFY_INFO* value,
                               TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMS_QUOTE_INFO(const TPMS_QUOTE_INFO* value,
                                 TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMS_QUOTE_INFO(TSS_SRC_DATA_BUF* buffer,
                             TPMS_QUOTE_INFO* value,
                             TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMS_COMMAND_AUDIT_INFO(
    const TPMS_COMMAND_AUDIT_INFO* value, TSS_DST_DATA_BUF* buffer);

TPM_RC
Parse_TPMS_COMMAND_AUDIT_INFO(TSS_SRC_DATA_BUF* buffer,
                              TPMS_COMMAND_AUDIT_INFO* value,
                              TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMS_SESSION_AUDIT_INFO(
    const TPMS_SESSION_AUDIT_INFO* value, TSS_DST_DATA_BUF* buffer);

TPM_RC
Parse_TPMS_SESSION_AUDIT_INFO(TSS_SRC_DATA_BUF* buffer,
                              TPMS_SESSION_AUDIT_INFO* value,
                              TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMS_CREATION_INFO(
    const TPMS_CREATION_INFO* value, TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMS_CREATION_INFO(TSS_SRC_DATA_BUF* buffer,
                                TPMS_CREATION_INFO* value,
                                TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMS_NV_CERTIFY_INFO(
    const TPMS_NV_CERTIFY_INFO* value, TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMS_NV_CERTIFY_INFO(TSS_SRC_DATA_BUF* buffer,
                                  TPMS_NV_CERTIFY_INFO* value,
                                  TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMS_ATTEST(const TPMS_ATTEST* value,
                             TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMS_ATTEST(TSS_SRC_DATA_BUF* buffer,
                         TPMS_ATTEST* value,
                         TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPM2B_ATTEST(const TPM2B_ATTEST* value,
                              TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPM2B_ATTEST(TSS_SRC_DATA_BUF* buffer,
                          TPM2B_ATTEST* value,
                          TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMS_AUTH_COMMAND(const TPMS_AUTH_COMMAND* value,
                                   TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMS_AUTH_COMMAND(TSS_SRC_DATA_BUF* buffer,
                               TPMS_AUTH_COMMAND* value,
                               TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMS_AUTH_RESPONSE(
    const TPMS_AUTH_RESPONSE* value, TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMS_AUTH_RESPONSE(TSS_SRC_DATA_BUF* buffer,
                                TPMS_AUTH_RESPONSE* value,
                                TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMT_SYM_DEF(const TPMT_SYM_DEF* value,
                              TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMT_SYM_DEF(TSS_SRC_DATA_BUF* buffer,
                          TPMT_SYM_DEF* value,
                          TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMT_SYM_DEF_OBJECT(
    const TPMT_SYM_DEF_OBJECT* value, TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMT_SYM_DEF_OBJECT(TSS_SRC_DATA_BUF* buffer,
                                 TPMT_SYM_DEF_OBJECT* value,
                                 TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPM2B_SYM_KEY(const TPM2B_SYM_KEY* value,
                               TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPM2B_SYM_KEY(TSS_SRC_DATA_BUF* buffer,
                           TPM2B_SYM_KEY* value,
                           TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMS_SYMCIPHER_PARMS(
    const TPMS_SYMCIPHER_PARMS* value, TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMS_SYMCIPHER_PARMS(TSS_SRC_DATA_BUF* buffer,
                                  TPMS_SYMCIPHER_PARMS* value,
                                  TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPM2B_SENSITIVE_DATA(
    const TPM2B_SENSITIVE_DATA* value, TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPM2B_SENSITIVE_DATA(TSS_SRC_DATA_BUF* buffer,
                                  TPM2B_SENSITIVE_DATA* value,
                                  TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMS_SENSITIVE_CREATE(
    const TPMS_SENSITIVE_CREATE* value, TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMS_SENSITIVE_CREATE(TSS_SRC_DATA_BUF* buffer,
                                   TPMS_SENSITIVE_CREATE* value,
                                   TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPM2B_SENSITIVE_CREATE(
    const TPM2B_SENSITIVE_CREATE* value, TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPM2B_SENSITIVE_CREATE(TSS_SRC_DATA_BUF* buffer,
                                    TPM2B_SENSITIVE_CREATE* value,
                                    TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMS_SCHEME_SIGHASH(
    const TPMS_SCHEME_SIGHASH* value, TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMS_SCHEME_SIGHASH(TSS_SRC_DATA_BUF* buffer,
                                 TPMS_SCHEME_SIGHASH* value,
                                 TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMS_SCHEME_XOR(const TPMS_SCHEME_XOR* value,
                                 TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMS_SCHEME_XOR(TSS_SRC_DATA_BUF* buffer,
                             TPMS_SCHEME_XOR* value,
                             TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMT_KEYEDHASH_SCHEME(
    const TPMT_KEYEDHASH_SCHEME* value, TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMT_KEYEDHASH_SCHEME(TSS_SRC_DATA_BUF* buffer,
                                   TPMT_KEYEDHASH_SCHEME* value,
                                   TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMS_SCHEME_ECDAA(const TPMS_SCHEME_ECDAA* value,
                                   TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMS_SCHEME_ECDAA(TSS_SRC_DATA_BUF* buffer,
                               TPMS_SCHEME_ECDAA* value,
                               TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMT_SIG_SCHEME(const TPMT_SIG_SCHEME* value,
                                 TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMT_SIG_SCHEME(TSS_SRC_DATA_BUF* buffer,
                             TPMT_SIG_SCHEME* value,
                             TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMS_SCHEME_OAEP(const TPMS_SCHEME_OAEP* value,
                                  TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMS_SCHEME_OAEP(TSS_SRC_DATA_BUF* buffer,
                              TPMS_SCHEME_OAEP* value,
                              TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMS_SCHEME_ECDH(const TPMS_SCHEME_ECDH* value,
                                  TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMS_SCHEME_ECDH(TSS_SRC_DATA_BUF* buffer,
                              TPMS_SCHEME_ECDH* value,
                              TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMS_SCHEME_MGF1(const TPMS_SCHEME_MGF1* value,
                                  TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMS_SCHEME_MGF1(TSS_SRC_DATA_BUF* buffer,
                              TPMS_SCHEME_MGF1* value,
                              TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMS_SCHEME_KDF1_SP800_56a(
    const TPMS_SCHEME_KDF1_SP800_56a* value, TSS_DST_DATA_BUF* buffer);

TPM_RC
Parse_TPMS_SCHEME_KDF1_SP800_56a(TSS_SRC_DATA_BUF* buffer,
                                 TPMS_SCHEME_KDF1_SP800_56a* value,
                                 TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMS_SCHEME_KDF2(const TPMS_SCHEME_KDF2* value,
                                  TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMS_SCHEME_KDF2(TSS_SRC_DATA_BUF* buffer,
                              TPMS_SCHEME_KDF2* value,
                              TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMS_SCHEME_KDF1_SP800_108(
    const TPMS_SCHEME_KDF1_SP800_108* value, TSS_DST_DATA_BUF* buffer);

TPM_RC
Parse_TPMS_SCHEME_KDF1_SP800_108(TSS_SRC_DATA_BUF* buffer,
                                 TPMS_SCHEME_KDF1_SP800_108* value,
                                 TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMT_KDF_SCHEME(const TPMT_KDF_SCHEME* value,
                                 TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMT_KDF_SCHEME(TSS_SRC_DATA_BUF* buffer,
                             TPMT_KDF_SCHEME* value,
                             TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMT_ASYM_SCHEME(const TPMT_ASYM_SCHEME* value,
                                  TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMT_ASYM_SCHEME(TSS_SRC_DATA_BUF* buffer,
                              TPMT_ASYM_SCHEME* value,
                              TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMT_RSA_SCHEME(const TPMT_RSA_SCHEME* value,
                                 TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMT_RSA_SCHEME(TSS_SRC_DATA_BUF* buffer,
                             TPMT_RSA_SCHEME* value,
                             TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMT_RSA_DECRYPT(const TPMT_RSA_DECRYPT* value,
                                  TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMT_RSA_DECRYPT(TSS_SRC_DATA_BUF* buffer,
                              TPMT_RSA_DECRYPT* value,
                              TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPM2B_PUBLIC_KEY_RSA(
    const TPM2B_PUBLIC_KEY_RSA* value, TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPM2B_PUBLIC_KEY_RSA(TSS_SRC_DATA_BUF* buffer,
                                  TPM2B_PUBLIC_KEY_RSA* value,
                                  TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPM2B_PRIVATE_KEY_RSA(
    const TPM2B_PRIVATE_KEY_RSA* value, TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPM2B_PRIVATE_KEY_RSA(TSS_SRC_DATA_BUF* buffer,
                                   TPM2B_PRIVATE_KEY_RSA* value,
                                   TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPM2B_ECC_PARAMETER(
    const TPM2B_ECC_PARAMETER* value, TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPM2B_ECC_PARAMETER(TSS_SRC_DATA_BUF* buffer,
                                 TPM2B_ECC_PARAMETER* value,
                                 TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMS_ECC_POINT(const TPMS_ECC_POINT* value,
                                TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMS_ECC_POINT(TSS_SRC_DATA_BUF* buffer,
                            TPMS_ECC_POINT* value,
                            TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPM2B_ECC_POINT(const TPM2B_ECC_POINT* value,
                                 TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPM2B_ECC_POINT(TSS_SRC_DATA_BUF* buffer,
                             TPM2B_ECC_POINT* value,
                             TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMT_ECC_SCHEME(const TPMT_ECC_SCHEME* value,
                                 TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMT_ECC_SCHEME(TSS_SRC_DATA_BUF* buffer,
                             TPMT_ECC_SCHEME* value,
                             TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMS_ALGORITHM_DETAIL_ECC(
    const TPMS_ALGORITHM_DETAIL_ECC* value, TSS_DST_DATA_BUF* buffer);

TPM_RC
Parse_TPMS_ALGORITHM_DETAIL_ECC(TSS_SRC_DATA_BUF* buffer,
                                TPMS_ALGORITHM_DETAIL_ECC* value,
                                TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMS_SIGNATURE_RSASSA(
    const TPMS_SIGNATURE_RSASSA* value, TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMS_SIGNATURE_RSASSA(TSS_SRC_DATA_BUF* buffer,
                                   TPMS_SIGNATURE_RSASSA* value,
                                   TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMS_SIGNATURE_RSAPSS(
    const TPMS_SIGNATURE_RSAPSS* value, TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMS_SIGNATURE_RSAPSS(TSS_SRC_DATA_BUF* buffer,
                                   TPMS_SIGNATURE_RSAPSS* value,
                                   TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMS_SIGNATURE_ECDSA(
    const TPMS_SIGNATURE_ECDSA* value, TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMS_SIGNATURE_ECDSA(TSS_SRC_DATA_BUF* buffer,
                                  TPMS_SIGNATURE_ECDSA* value,
                                  TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMT_SIGNATURE(const TPMT_SIGNATURE* value,
                                TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMT_SIGNATURE(TSS_SRC_DATA_BUF* buffer,
                            TPMT_SIGNATURE* value,
                            TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPM2B_ENCRYPTED_SECRET(
    const TPM2B_ENCRYPTED_SECRET* value, TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPM2B_ENCRYPTED_SECRET(TSS_SRC_DATA_BUF* buffer,
                                    TPM2B_ENCRYPTED_SECRET* value,
                                    TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMS_KEYEDHASH_PARMS(
    const TPMS_KEYEDHASH_PARMS* value, TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMS_KEYEDHASH_PARMS(TSS_SRC_DATA_BUF* buffer,
                                  TPMS_KEYEDHASH_PARMS* value,
                                  TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMS_ASYM_PARMS(const TPMS_ASYM_PARMS* value,
                                 TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMS_ASYM_PARMS(TSS_SRC_DATA_BUF* buffer,
                             TPMS_ASYM_PARMS* value,
                             TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMS_RSA_PARMS(const TPMS_RSA_PARMS* value,
                                TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMS_RSA_PARMS(TSS_SRC_DATA_BUF* buffer,
                            TPMS_RSA_PARMS* value,
                            TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMS_ECC_PARMS(const TPMS_ECC_PARMS* value,
                                TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMS_ECC_PARMS(TSS_SRC_DATA_BUF* buffer,
                            TPMS_ECC_PARMS* value,
                            TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMT_PUBLIC_PARMS(const TPMT_PUBLIC_PARMS* value,
                                   TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMT_PUBLIC_PARMS(TSS_SRC_DATA_BUF* buffer,
                               TPMT_PUBLIC_PARMS* value,
                               TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMT_PUBLIC(const TPMT_PUBLIC* value,
                             TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMT_PUBLIC(TSS_SRC_DATA_BUF* buffer,
                         TPMT_PUBLIC* value,
                         TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPM2B_PUBLIC(const TPM2B_PUBLIC* value,
                              TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPM2B_PUBLIC(TSS_SRC_DATA_BUF* buffer,
                          TPM2B_PUBLIC* value,
                          TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPM2B_PRIVATE_VENDOR_SPECIFIC(
    const TPM2B_PRIVATE_VENDOR_SPECIFIC* value, TSS_DST_DATA_BUF* buffer);

TPM_RC
Parse_TPM2B_PRIVATE_VENDOR_SPECIFIC(TSS_SRC_DATA_BUF* buffer,
                                    TPM2B_PRIVATE_VENDOR_SPECIFIC* value,
                                    TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMT_SENSITIVE(const TPMT_SENSITIVE* value,
                                TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMT_SENSITIVE(TSS_SRC_DATA_BUF* buffer,
                            TPMT_SENSITIVE* value,
                            TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPM2B_SENSITIVE(const TPM2B_SENSITIVE* value,
                                 TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPM2B_SENSITIVE(TSS_SRC_DATA_BUF* buffer,
                             TPM2B_SENSITIVE* value,
                             TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize__PRIVATE(const _PRIVATE* value,
                          TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse__PRIVATE(TSS_SRC_DATA_BUF* buffer,
                      _PRIVATE* value,
                      TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPM2B_PRIVATE(const TPM2B_PRIVATE* value,
                               TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPM2B_PRIVATE(TSS_SRC_DATA_BUF* buffer,
                           TPM2B_PRIVATE* value,
                           TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize__ID_OBJECT(const _ID_OBJECT* value,
                            TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse__ID_OBJECT(TSS_SRC_DATA_BUF* buffer,
                        _ID_OBJECT* value,
                        TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPM2B_ID_OBJECT(const TPM2B_ID_OBJECT* value,
                                 TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPM2B_ID_OBJECT(TSS_SRC_DATA_BUF* buffer,
                             TPM2B_ID_OBJECT* value,
                             TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMS_NV_PUBLIC(const TPMS_NV_PUBLIC* value,
                                TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMS_NV_PUBLIC(TSS_SRC_DATA_BUF* buffer,
                            TPMS_NV_PUBLIC* value,
                            TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPM2B_NV_PUBLIC(const TPM2B_NV_PUBLIC* value,
                                 TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPM2B_NV_PUBLIC(TSS_SRC_DATA_BUF* buffer,
                             TPM2B_NV_PUBLIC* value,
                             TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPM2B_CONTEXT_SENSITIVE(
    const TPM2B_CONTEXT_SENSITIVE* value, TSS_DST_DATA_BUF* buffer);

TPM_RC
Parse_TPM2B_CONTEXT_SENSITIVE(TSS_SRC_DATA_BUF* buffer,
                              TPM2B_CONTEXT_SENSITIVE* value,
                              TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMS_CONTEXT_DATA(const TPMS_CONTEXT_DATA* value,
                                   TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMS_CONTEXT_DATA(TSS_SRC_DATA_BUF* buffer,
                               TPMS_CONTEXT_DATA* value,
                               TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPM2B_CONTEXT_DATA(
    const TPM2B_CONTEXT_DATA* value, TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPM2B_CONTEXT_DATA(TSS_SRC_DATA_BUF* buffer,
                                TPM2B_CONTEXT_DATA* value,
                                TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMS_CONTEXT(const TPMS_CONTEXT* value,
                              TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMS_CONTEXT(TSS_SRC_DATA_BUF* buffer,
                          TPMS_CONTEXT* value,
                          TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMS_CREATION_DATA(
    const TPMS_CREATION_DATA* value, TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMS_CREATION_DATA(TSS_SRC_DATA_BUF* buffer,
                                TPMS_CREATION_DATA* value,
                                TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPM2B_CREATION_DATA(
    const TPM2B_CREATION_DATA* value, TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPM2B_CREATION_DATA(TSS_SRC_DATA_BUF* buffer,
                                 TPM2B_CREATION_DATA* value,
                                 TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMU_HA(const TPMU_HA* value,
                         TPMI_ALG_HASH selector,
                         TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMU_HA(TSS_SRC_DATA_BUF* buffer,
                     TPMI_ALG_HASH selector,
                     TPMU_HA* value,
                     TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMU_NAME(const TPMU_NAME* value,
                           TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMU_NAME(TSS_SRC_DATA_BUF* buffer,
                       TPMU_NAME* value,
                       TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMU_CAPABILITIES(const TPMU_CAPABILITIES* value,
                                   TPM_CAP selector,
                                   TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMU_CAPABILITIES(TSS_SRC_DATA_BUF* buffer,
                               TPM_CAP selector,
                               TPMU_CAPABILITIES* value,
                               TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMU_ATTEST(const TPMU_ATTEST* value,
                             TPMI_ST_ATTEST selector,
                             TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMU_ATTEST(TSS_SRC_DATA_BUF* buffer,
                             TPMI_ST_ATTEST selector,
                         TPMU_ATTEST* value,
                         TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMU_SYM_KEY_BITS(const TPMU_SYM_KEY_BITS* value,
                                   TPMI_ALG_SYM selector,
                                   TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMU_SYM_KEY_BITS(TSS_SRC_DATA_BUF* buffer,
                                   TPMI_ALG_SYM selector,
                               TPMU_SYM_KEY_BITS* value,
                               TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMU_SYM_MODE(const TPMU_SYM_MODE* value,
                               TPMI_ALG_SYM selector,
                               TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMU_SYM_MODE(TSS_SRC_DATA_BUF* buffer,
                               TPMI_ALG_SYM selector,
                           TPMU_SYM_MODE* value,
                           TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMU_SCHEME_KEYEDHASH(
        const TPMU_SCHEME_KEYEDHASH* value,
        TPMI_ALG_KEYEDHASH_SCHEME selector,
        TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMU_SCHEME_KEYEDHASH(TSS_SRC_DATA_BUF* buffer,
                                       TPMI_ALG_KEYEDHASH_SCHEME selector,
                                   TPMU_SCHEME_KEYEDHASH* value,
                                   TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMU_SIG_SCHEME(const TPMU_SIG_SCHEME* value,
                                 TPMI_ALG_SIG_SCHEME selector,
                                 TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMU_SIG_SCHEME(TSS_SRC_DATA_BUF* buffer,
                                 TPMI_ALG_SIG_SCHEME selector,
                             TPMU_SIG_SCHEME* value,
                             TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMU_KDF_SCHEME(const TPMU_KDF_SCHEME* value,
                                 TPMI_ALG_KDF selector,
                                 TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMU_KDF_SCHEME(TSS_SRC_DATA_BUF* buffer,
                                 TPMI_ALG_KDF selector,
                             TPMU_KDF_SCHEME* value,
                             TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMU_ASYM_SCHEME(const TPMU_ASYM_SCHEME* value,
                                  TPMI_ALG_ASYM_SCHEME selector,
                                  TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMU_ASYM_SCHEME(TSS_SRC_DATA_BUF* buffer,
                                  TPMI_ALG_ASYM_SCHEME selector,
                              TPMU_ASYM_SCHEME* value,
                              TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMU_SIGNATURE(const TPMU_SIGNATURE* value,
                                TPMI_ALG_SIG_SCHEME selector,
                                TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMU_SIGNATURE(TSS_SRC_DATA_BUF* buffer,
                                TPMI_ALG_SIG_SCHEME selector,
                            TPMU_SIGNATURE* value,
                            TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMU_ENCRYPTED_SECRET(
    const TPMU_ENCRYPTED_SECRET* value, TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMU_ENCRYPTED_SECRET(TSS_SRC_DATA_BUF* buffer,
                                   TPMU_ENCRYPTED_SECRET* value,
                                   TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMU_PUBLIC_ID(const TPMU_PUBLIC_ID* value,
                                TPMI_ALG_PUBLIC selector,
                                TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMU_PUBLIC_ID(TSS_SRC_DATA_BUF* buffer,
                                TPMI_ALG_PUBLIC selector,
                            TPMU_PUBLIC_ID* value,
                            TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMU_PUBLIC_PARMS(const TPMU_PUBLIC_PARMS* value,
                                   TPMI_ALG_PUBLIC selector,
                                   TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMU_PUBLIC_PARMS(TSS_SRC_DATA_BUF* buffer,
                                   TPMI_ALG_PUBLIC selector,
                               TPMU_PUBLIC_PARMS* value,
                               TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMU_SENSITIVE_COMPOSITE(
        const TPMU_SENSITIVE_COMPOSITE* value,
        TPMI_ALG_PUBLIC selector,
        TSS_DST_DATA_BUF* buffer);

TPM_RC
Parse_TPMU_SENSITIVE_COMPOSITE(TSS_SRC_DATA_BUF* buffer,
                               TPMI_ALG_PUBLIC selector,
                               TPMU_SENSITIVE_COMPOSITE* value,
                               TSS_DST_DATA_BUF* value_bytes);

TPM_RC tss_Serialize_TPMU_SYM_DETAILS(const TPMU_SYM_DETAILS* value,
                                  TPMI_ALG_SYM selector,
                                  TSS_DST_DATA_BUF* buffer);

TPM_RC tss_Parse_TPMU_SYM_DETAILS(TSS_SRC_DATA_BUF* buffer,
                                  TPMI_ALG_SYM selector,
                              TPMU_SYM_DETAILS* value,
                              TSS_DST_DATA_BUF* value_bytes);

#endif  // MINI_TRUNKS_TSS_SERDE_H_
