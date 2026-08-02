// Copyright 2021 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_TRUNKS_TSS_TYPES_H_
#define MINI_TRUNKS_TSS_TYPES_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#if !defined(SHA1_DIGEST_SIZE)
#define SHA1_DIGEST_SIZE 20
#endif
#if !defined(SHA1_BLOCK_SIZE)
#define SHA1_BLOCK_SIZE 64
#endif
#if !defined(SHA1_DER_SIZE)
#define SHA1_DER_SIZE 15
#endif
#if !defined(SHA1_DER)
#define SHA1_DER                                                            \
  {                                                                         \
    0x30, 0x21, 0x30, 0x09, 0x06, 0x05, 0x2B, 0x0E, 0x03, 0x02, 0x1A, 0x05, \
        0x00, 0x04, 0x14                                                    \
  }
#endif
#if !defined(SHA256_DIGEST_SIZE)
#define SHA256_DIGEST_SIZE 32
#endif
#if !defined(SHA256_BLOCK_SIZE)
#define SHA256_BLOCK_SIZE 64
#endif
#if !defined(SHA256_DER_SIZE)
#define SHA256_DER_SIZE 19
#endif
#if !defined(SHA256_DER)
#define SHA256_DER                                                          \
  {                                                                         \
    0x30, 0x31, 0x30, 0x0d, 0x06, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, \
        0x04, 0x02, 0x01, 0x05, 0x00, 0x04, 0x20                            \
  }
#endif
#if !defined(SHA384_DIGEST_SIZE)
#define SHA384_DIGEST_SIZE 48
#endif
#if !defined(SHA384_BLOCK_SIZE)
#define SHA384_BLOCK_SIZE 128
#endif
#if !defined(SHA384_DER_SIZE)
#define SHA384_DER_SIZE 19
#endif
#if !defined(SHA384_DER)
#define SHA384_DER                                                          \
  {                                                                         \
    0x30, 0x41, 0x30, 0x0d, 0x06, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, \
        0x04, 0x02, 0x02, 0x05, 0x00, 0x04, 0x30                            \
  }
#endif
#if !defined(SHA512_DIGEST_SIZE)
#define SHA512_DIGEST_SIZE 64
#endif
#if !defined(SHA512_BLOCK_SIZE)
#define SHA512_BLOCK_SIZE 128
#endif
#if !defined(SHA512_DER_SIZE)
#define SHA512_DER_SIZE 19
#endif
#if !defined(SHA512_DER)
#define SHA512_DER                                                          \
  {                                                                         \
    0x30, 0x51, 0x30, 0x0d, 0x06, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, \
        0x04, 0x02, 0x03, 0x05, 0x00, 0x04, 0x40                            \
  }
#endif
#if !defined(SM3_256_DIGEST_SIZE)
#define SM3_256_DIGEST_SIZE 32
#endif
#if !defined(SM3_256_BLOCK_SIZE)
#define SM3_256_BLOCK_SIZE 64
#endif
#if !defined(SM3_256_DER_SIZE)
#define SM3_256_DER_SIZE 18
#endif
#if !defined(SM3_256_DER)
#define SM3_256_DER                                                         \
  {                                                                         \
    0x30, 0x30, 0x30, 0x0c, 0x06, 0x08, 0x2a, 0x81, 0x1c, 0x81, 0x45, 0x01, \
        0x83, 0x11, 0x05, 0x00, 0x04, 0x20                                  \
  }
#endif
#if !defined(MAX_SESSION_NUMBER)
#define MAX_SESSION_NUMBER 3
#endif
#if !defined(YES)
#define YES 1
#endif
#if !defined(NO)
#define NO 0
#endif
#if !defined(TRUE)
#define TRUE 1
#endif
#if !defined(FALSE)
#define FALSE 0
#endif
#if !defined(SET)
#define SET 1
#endif
#if !defined(CLEAR)
#define CLEAR 0
#endif
#if !defined(BIG_ENDIAN_TPM)
#define BIG_ENDIAN_TPM NO
#endif
#if !defined(LITTLE_ENDIAN_TPM)
#define LITTLE_ENDIAN_TPM YES
#endif
#if !defined(NO_AUTO_ALIGN)
#define NO_AUTO_ALIGN NO
#endif
#if !defined(RSA_KEY_SIZES_BITS)
#define RSA_KEY_SIZES_BITS \
  { 1024, 2048 }
#endif
#if !defined(MAX_RSA_KEY_BITS)
#define MAX_RSA_KEY_BITS 2048
#endif
#if !defined(MAX_RSA_KEY_BYTES)
#define MAX_RSA_KEY_BYTES ((MAX_RSA_KEY_BITS + 7) / 8)
#endif
#if !defined(ECC_CURVES)
#define ECC_CURVES                                      \
  {                                                     \
    TPM_ECC_NIST_P256, TPM_ECC_BN_P256, \
        TPM_ECC_SM2_P256                        \
  }
#endif
#if !defined(ECC_KEY_SIZES_BITS)
#define ECC_KEY_SIZES_BITS \
  { 256 }
#endif
#if !defined(MAX_ECC_KEY_BITS)
#define MAX_ECC_KEY_BITS 256
#endif
#if !defined(MAX_ECC_KEY_BYTES)
#define MAX_ECC_KEY_BYTES ((MAX_ECC_KEY_BITS + 7) / 8)
#endif
#if !defined(AES_KEY_SIZES_BITS)
#define AES_KEY_SIZES_BITS \
  { 128 }
#endif
#if !defined(MAX_AES_KEY_BITS)
#define MAX_AES_KEY_BITS 128
#endif
#if !defined(MAX_AES_BLOCK_SIZE_BYTES)
#define MAX_AES_BLOCK_SIZE_BYTES 16
#endif
#if !defined(MAX_AES_KEY_BYTES)
#define MAX_AES_KEY_BYTES ((MAX_AES_KEY_BITS + 7) / 8)
#endif
#if !defined(SM4_KEY_SIZES_BITS)
#define SM4_KEY_SIZES_BITS \
  { 128 }
#endif
#if !defined(MAX_SM4_KEY_BITS)
#define MAX_SM4_KEY_BITS 128
#endif
#if !defined(MAX_SM4_BLOCK_SIZE_BYTES)
#define MAX_SM4_BLOCK_SIZE_BYTES 16
#endif
#if !defined(MAX_SM4_KEY_BYTES)
#define MAX_SM4_KEY_BYTES ((MAX_SM4_KEY_BITS + 7) / 8)
#endif
#if !defined(MAX_SYM_KEY_BITS)
#define MAX_SYM_KEY_BITS MAX_AES_KEY_BITS
#endif
#if !defined(MAX_SYM_KEY_BYTES)
#define MAX_SYM_KEY_BYTES MAX_AES_KEY_BYTES
#endif
#if !defined(MAX_SYM_BLOCK_SIZE)
#define MAX_SYM_BLOCK_SIZE MAX_AES_BLOCK_SIZE_BYTES
#endif
#if !defined(FIELD_UPGRADE_IMPLEMENTED)
#define FIELD_UPGRADE_IMPLEMENTED NO
#endif
#if !defined(BSIZE)
#define BSIZE UINT16
#endif
#if !defined(BUFFER_ALIGNMENT)
#define BUFFER_ALIGNMENT 4
#endif
#if !defined(IMPLEMENTATION_PCR)
#define IMPLEMENTATION_PCR 24
#endif
#if !defined(PLATFORM_PCR)
#define PLATFORM_PCR 24
#endif
#if !defined(DRTM_PCR)
#define DRTM_PCR 17
#endif
#if !defined(HCRTM_PCR)
#define HCRTM_PCR 0
#endif
#if !defined(NUM_LOCALITIES)
#define NUM_LOCALITIES 5
#endif
#if !defined(MAX_HANDLE_NUM)
#define MAX_HANDLE_NUM 3
#endif
#if !defined(MAX_ACTIVE_SESSIONS)
#define MAX_ACTIVE_SESSIONS 64
#endif
#if !defined(CONTEXT_SLOT)
#define CONTEXT_SLOT UINT16
#endif
#if !defined(CONTEXT_COUNTER)
#define CONTEXT_COUNTER UINT64
#endif
#if !defined(MAX_LOADED_SESSIONS)
#define MAX_LOADED_SESSIONS 3
#endif
#if !defined(MAX_SESSION_NUM)
#define MAX_SESSION_NUM 3
#endif
#if !defined(MAX_LOADED_OBJECTS)
#define MAX_LOADED_OBJECTS 3
#endif
#if !defined(MIN_EVICT_OBJECTS)
#define MIN_EVICT_OBJECTS 2
#endif
#if !defined(PCR_SELECT_MIN)
#define PCR_SELECT_MIN ((PLATFORM_PCR + 7) / 8)
#endif
#if !defined(PCR_SELECT_MAX)
#define PCR_SELECT_MAX ((IMPLEMENTATION_PCR + 7) / 8)
#endif
#if !defined(NUM_POLICY_PCR_GROUP)
#define NUM_POLICY_PCR_GROUP 1
#endif
#if !defined(NUM_AUTHVALUE_PCR_GROUP)
#define NUM_AUTHVALUE_PCR_GROUP 1
#endif
#if !defined(MAX_CONTEXT_SIZE)
#define MAX_CONTEXT_SIZE 4000
#endif
#if !defined(MAX_DIGEST_BUFFER)
#define MAX_DIGEST_BUFFER 1024
#endif
#if !defined(MAX_NV_INDEX_SIZE)
#define MAX_NV_INDEX_SIZE 2048
#endif
#if !defined(MAX_NV_BUFFER_SIZE)
#define MAX_NV_BUFFER_SIZE 1024
#endif
#if !defined(MAX_CAP_BUFFER)
#define MAX_CAP_BUFFER 1024
#endif
#if !defined(NV_MEMORY_SIZE)
#define NV_MEMORY_SIZE 16384
#endif
#if !defined(NUM_STATIC_PCR)
#define NUM_STATIC_PCR 16
#endif
#if !defined(MAX_ALG_LIST_SIZE)
#define MAX_ALG_LIST_SIZE 64
#endif
#if !defined(TIMER_PRESCALE)
#define TIMER_PRESCALE 100000
#endif
#if !defined(PRIMARY_SEED_SIZE)
#define PRIMARY_SEED_SIZE 32
#endif
#if !defined(CONTEXT_ENCRYPT_ALG)
#define CONTEXT_ENCRYPT_ALG TPM_ALG_AES
#endif
#if !defined(CONTEXT_ENCRYPT_KEY_BITS)
#define CONTEXT_ENCRYPT_KEY_BITS MAX_SYM_KEY_BITS
#endif
#if !defined(CONTEXT_ENCRYPT_KEY_BYTES)
#define CONTEXT_ENCRYPT_KEY_BYTES ((CONTEXT_ENCRYPT_KEY_BITS + 7) / 8)
#endif
#if !defined(CONTEXT_INTEGRITY_HASH_ALG)
#define CONTEXT_INTEGRITY_HASH_ALG TPM_ALG_SHA256
#endif
#if !defined(CONTEXT_INTEGRITY_HASH_SIZE)
#define CONTEXT_INTEGRITY_HASH_SIZE SHA256_DIGEST_SIZE
#endif
#if !defined(PROOF_SIZE)
#define PROOF_SIZE CONTEXT_INTEGRITY_HASH_SIZE
#endif
#if !defined(NV_CLOCK_UPDATE_INTERVAL)
#define NV_CLOCK_UPDATE_INTERVAL 12
#endif
#if !defined(NUM_POLICY_PCR)
#define NUM_POLICY_PCR 1
#endif
#if !defined(MAX_COMMAND_SIZE)
#define MAX_COMMAND_SIZE 4096
#endif
#if !defined(MAX_RESPONSE_SIZE)
#define MAX_RESPONSE_SIZE 4096
#endif
#if !defined(ORDERLY_BITS)
#define ORDERLY_BITS 8
#endif
#if !defined(MAX_ORDERLY_COUNT)
#define MAX_ORDERLY_COUNT ((1 << ORDERLY_BITS) - 1)
#endif
#if !defined(ALG_ID_FIRST)
#define ALG_ID_FIRST TPM_ALG_FIRST
#endif
#if !defined(ALG_ID_LAST)
#define ALG_ID_LAST TPM_ALG_LAST
#endif
#if !defined(MAX_SYM_DATA)
#define MAX_SYM_DATA 128
#endif
#if !defined(MAX_RNG_ENTROPY_SIZE)
#define MAX_RNG_ENTROPY_SIZE 64
#endif
#if !defined(RAM_INDEX_SPACE)
#define RAM_INDEX_SPACE 512
#endif
#if !defined(RSA_DEFAULT_PUBLIC_EXPONENT)
#define RSA_DEFAULT_PUBLIC_EXPONENT 0x00010001
#endif
#if !defined(ENABLE_PCR_NO_INCREMENT)
#define ENABLE_PCR_NO_INCREMENT YES
#endif
#if !defined(CRT_FORMAT_RSA)
#define CRT_FORMAT_RSA YES
#endif
#if !defined(PRIVATE_VENDOR_SPECIFIC_BYTES)
#define PRIVATE_VENDOR_SPECIFIC_BYTES \
  ((MAX_RSA_KEY_BYTES / 2) * (3 + CRT_FORMAT_RSA * 2))
#endif
#if !defined(MAX_CAP_DATA)
#define MAX_CAP_DATA \
  (MAX_CAP_BUFFER - sizeof(TPM_CAP) - sizeof(UINT32))
#endif
#if !defined(MAX_CAP_ALGS)
#define MAX_CAP_ALGS (TPM_ALG_LAST - TPM_ALG_FIRST + 1)
#endif
#if !defined(MAX_CAP_HANDLES)
#define MAX_CAP_HANDLES (MAX_CAP_DATA / sizeof(TPM_HANDLE))
#endif
#if !defined(MAX_CAP_CC)
#define MAX_CAP_CC ((TPM_CC_LAST - TPM_CC_FIRST) + 1)
#endif
#if !defined(MAX_CAP_CCE)
#define MAX_CAP_CCE ((TPM_CCE_LAST - TPM_CCE_FIRST) + 1)
#endif
#if !defined(MAX_CAP_CC_ALL)
#define MAX_CAP_CC_ALL (MAX_CAP_CC + MAX_CAP_CCE)
#endif
#if !defined(MAX_TPM_PROPERTIES)
#define MAX_TPM_PROPERTIES (MAX_CAP_DATA / sizeof(TPMS_TAGGED_PROPERTY))
#endif
#if !defined(MAX_PCR_PROPERTIES)
#define MAX_PCR_PROPERTIES \
  (MAX_CAP_DATA / sizeof(TPMS_TAGGED_PCR_SELECT))
#endif
#if !defined(MAX_ECC_CURVES)
#define MAX_ECC_CURVES (MAX_CAP_DATA / sizeof(TPM_ECC_CURVE))
#endif
#if !defined(HASH_COUNT)
#define HASH_COUNT 5
#endif

#define IS_TPM2_STD_CMD(x) \
  ((x) >= TPM_CC_FIRST && (x) <= TPM_CC_LAST)
#define IS_TPM2_EXT_CMD(x) \
  ((x) >= TPM_CCE_FIRST && (x) <= TPM_CCE_LAST)
#define IS_TPM2_CMD(x) (IS_TPM2_STD_CMD(x) || IS_TPM2_EXT_CMD(x))

typedef uint8_t UINT8;
typedef uint8_t BYTE;
typedef int8_t INT8;
typedef int BOOL;
typedef uint16_t UINT16;
typedef int16_t INT16;
typedef uint32_t UINT32;
typedef int32_t INT32;
typedef uint64_t UINT64;
typedef int64_t INT64;
typedef UINT32 TPM_ALGORITHM_ID;
typedef UINT32 TPM_MODIFIER_INDICATOR;
typedef UINT32 TPM_AUTHORIZATION_SIZE;
typedef UINT32 TPM_PARAMETER_SIZE;
typedef UINT16 TPM_KEY_SIZE;
typedef UINT16 TPM_KEY_BITS;
typedef UINT32 TPM_HANDLE;
struct TPM2B_DIGEST;
typedef struct TPM2B_DIGEST TPM2B_NONCE;
typedef struct TPM2B_DIGEST TPM2B_AUTH;
typedef struct TPM2B_DIGEST TPM2B_OPERAND;
struct TPMS_SCHEME_SIGHASH;
typedef struct TPMS_SCHEME_SIGHASH TPMS_SCHEME_HMAC;
typedef struct TPMS_SCHEME_SIGHASH TPMS_SCHEME_RSASSA;
typedef struct TPMS_SCHEME_SIGHASH TPMS_SCHEME_RSAPSS;
typedef struct TPMS_SCHEME_SIGHASH TPMS_SCHEME_ECDSA;
typedef struct TPMS_SCHEME_SIGHASH TPMS_SCHEME_SM2;
typedef struct TPMS_SCHEME_SIGHASH TPMS_SCHEME_ECSCHNORR;
typedef BYTE TPMI_YES_NO;
typedef TPM_HANDLE TPMI_DH_OBJECT;
typedef TPM_HANDLE TPMI_DH_PERSISTENT;
typedef TPM_HANDLE TPMI_DH_ENTITY;
typedef TPM_HANDLE TPMI_DH_PCR;
typedef TPM_HANDLE TPMI_SH_AUTH_SESSION;
typedef TPM_HANDLE TPMI_SH_HMAC;
typedef TPM_HANDLE TPMI_SH_POLICY;
typedef TPM_HANDLE TPMI_DH_CONTEXT;
typedef TPM_HANDLE TPMI_RH_HIERARCHY;
typedef TPM_HANDLE TPMI_RH_ENABLES;
typedef TPM_HANDLE TPMI_RH_HIERARCHY_AUTH;
typedef TPM_HANDLE TPMI_RH_PLATFORM;
typedef TPM_HANDLE TPMI_RH_OWNER;
typedef TPM_HANDLE TPMI_RH_ENDORSEMENT;
typedef TPM_HANDLE TPMI_RH_PROVISION;
typedef TPM_HANDLE TPMI_RH_CLEAR;
typedef TPM_HANDLE TPMI_RH_NV_AUTH;
typedef TPM_HANDLE TPMI_RH_LOCKOUT;
typedef TPM_HANDLE TPMI_RH_NV_INDEX;
typedef UINT16 TPM_ALG_ID;
typedef TPM_ALG_ID TPMI_ALG_HASH;
typedef TPM_ALG_ID TPMI_ALG_ASYM;
typedef TPM_ALG_ID TPMI_ALG_SYM;
typedef TPM_ALG_ID TPMI_ALG_SYM_OBJECT;
typedef TPM_ALG_ID TPMI_ALG_SYM_MODE;
typedef TPM_ALG_ID TPMI_ALG_KDF;
typedef TPM_ALG_ID TPMI_ALG_SIG_SCHEME;
typedef TPM_ALG_ID TPMI_ECC_KEY_EXCHANGE;
typedef UINT16 TPM_ST;
typedef TPM_ST TPMI_ST_COMMAND_TAG;
typedef TPM_ST TPMI_ST_ATTEST;
typedef TPM_KEY_BITS TPMI_AES_KEY_BITS;
typedef TPM_KEY_BITS TPMI_SM4_KEY_BITS;
typedef TPM_ALG_ID TPMI_ALG_KEYEDHASH_SCHEME;
typedef TPM_ALG_ID TPMI_ALG_ASYM_SCHEME;
typedef TPM_ALG_ID TPMI_ALG_RSA_SCHEME;
typedef TPM_ALG_ID TPMI_ALG_RSA_DECRYPT;
typedef TPM_KEY_BITS TPMI_RSA_KEY_BITS;
typedef TPM_ALG_ID TPMI_ALG_ECC_SCHEME;
typedef UINT16 TPM_ECC_CURVE;
typedef TPM_ECC_CURVE TPMI_ECC_CURVE;
typedef TPM_ALG_ID TPMI_ALG_PUBLIC;
typedef UINT32 TPMA_ALGORITHM;
typedef UINT32 TPMA_OBJECT;
typedef UINT8 TPMA_SESSION;
typedef UINT8 TPMA_LOCALITY;
typedef UINT32 TPMA_PERMANENT;
typedef UINT32 TPMA_STARTUP_CLEAR;
typedef UINT32 TPMA_MEMORY;
typedef UINT32 TPM_CC;
typedef TPM_CC TPMA_CC;
typedef UINT32 TPM_NV_INDEX;
typedef UINT32 TPMA_NV;
typedef UINT32 TPM_SPEC;
typedef UINT32 TPM_GENERATED;
typedef UINT32 TPM_RC;
typedef INT8 TPM_CLOCK_ADJUST;
typedef UINT16 TPM_EO;
typedef UINT16 TPM_SU;
typedef UINT8 TPM_SE;
typedef UINT32 TPM_CAP;
typedef UINT32 TPM_PT;
typedef UINT32 TPM_PT_PCR;
typedef UINT32 TPM_PS;
typedef UINT8 TPM_HT;
typedef UINT32 TPM_RH;
typedef TPM_HANDLE TPM_HC;

#define TPM_SPEC_FAMILY ((TPM_SPEC)(0x322E3000))
#define TPM_SPEC_LEVEL ((TPM_SPEC)(00))
#define TPM_SPEC_VERSION ((TPM_SPEC)(99))
#define TPM_SPEC_YEAR ((TPM_SPEC)(2013))
#define TPM_SPEC_DAY_OF_YEAR ((TPM_SPEC)(304))
#define TPM_GENERATED_VALUE ((TPM_GENERATED)(0xff544347))
#define TPM_ALG_ERROR ((TPM_ALG_ID)(0x0000))
#define TPM_ALG_FIRST ((TPM_ALG_ID)(0x0001))
#define TPM_ALG_RSA ((TPM_ALG_ID)(0x0001))
#define TPM_ALG_SHA ((TPM_ALG_ID)(0x0004))
#define TPM_ALG_SHA1 ((TPM_ALG_ID)(0x0004))
#define TPM_ALG_HMAC ((TPM_ALG_ID)(0x0005))
#define TPM_ALG_AES ((TPM_ALG_ID)(0x0006))
#define TPM_ALG_MGF1 ((TPM_ALG_ID)(0x0007))
#define TPM_ALG_KEYEDHASH ((TPM_ALG_ID)(0x0008))
#define TPM_ALG_XOR ((TPM_ALG_ID)(0x000A))
#define TPM_ALG_SHA256 ((TPM_ALG_ID)(0x000B))
#define TPM_ALG_SHA384 ((TPM_ALG_ID)(0x000C))
#define TPM_ALG_SHA512 ((TPM_ALG_ID)(0x000D))
#define TPM_ALG_NULL ((TPM_ALG_ID)(0x0010))
#define TPM_ALG_SM3_256 ((TPM_ALG_ID)(0x0012))
#define TPM_ALG_SM4 ((TPM_ALG_ID)(0x0013))
#define TPM_ALG_RSASSA ((TPM_ALG_ID)(0x0014))
#define TPM_ALG_RSAES ((TPM_ALG_ID)(0x0015))
#define TPM_ALG_RSAPSS ((TPM_ALG_ID)(0x0016))
#define TPM_ALG_OAEP ((TPM_ALG_ID)(0x0017))
#define TPM_ALG_ECDSA ((TPM_ALG_ID)(0x0018))
#define TPM_ALG_ECDH ((TPM_ALG_ID)(0x0019))
#define TPM_ALG_ECDAA ((TPM_ALG_ID)(0x001A))
#define TPM_ALG_SM2 ((TPM_ALG_ID)(0x001B))
#define TPM_ALG_ECSCHNORR ((TPM_ALG_ID)(0x001C))
#define TPM_ALG_ECMQV ((TPM_ALG_ID)(0x001D))
#define TPM_ALG_KDF1_SP800_56a ((TPM_ALG_ID)(0x0020))
#define TPM_ALG_KDF2 ((TPM_ALG_ID)(0x0021))
#define TPM_ALG_KDF1_SP800_108 ((TPM_ALG_ID)(0x0022))
#define TPM_ALG_ECC ((TPM_ALG_ID)(0x0023))
#define TPM_ALG_SYMCIPHER ((TPM_ALG_ID)(0x0025))
#define TPM_ALG_CTR ((TPM_ALG_ID)(0x0040))
#define TPM_ALG_OFB ((TPM_ALG_ID)(0x0041))
#define TPM_ALG_CBC ((TPM_ALG_ID)(0x0042))
#define TPM_ALG_CFB ((TPM_ALG_ID)(0x0043))
#define TPM_ALG_ECB ((TPM_ALG_ID)(0x0044))
#define TPM_ALG_LAST ((TPM_ALG_ID)(0x0044))
#define TPM_ECC_NONE ((TPM_ECC_CURVE)(0x0000))
#define TPM_ECC_NIST_P192 ((TPM_ECC_CURVE)(0x0001))
#define TPM_ECC_NIST_P224 ((TPM_ECC_CURVE)(0x0002))
#define TPM_ECC_NIST_P256 ((TPM_ECC_CURVE)(0x0003))
#define TPM_ECC_NIST_P384 ((TPM_ECC_CURVE)(0x0004))
#define TPM_ECC_NIST_P521 ((TPM_ECC_CURVE)(0x0005))
#define TPM_ECC_BN_P256 ((TPM_ECC_CURVE)(0x0010))
#define TPM_ECC_BN_P638 ((TPM_ECC_CURVE)(0x0011))
#define TPM_ECC_SM2_P256 ((TPM_ECC_CURVE)(0x0020))
#define TPM_CC_FIRST ((TPM_CC)(0x0000011F))
#define TPM_CC_PP_FIRST ((TPM_CC)(0x0000011F))
#define TPM_CC_NV_UndefineSpaceSpecial ((TPM_CC)(0x0000011F))
#define TPM_CC_EvictControl ((TPM_CC)(0x00000120))
#define TPM_CC_HierarchyControl ((TPM_CC)(0x00000121))
#define TPM_CC_NV_UndefineSpace ((TPM_CC)(0x00000122))
#define TPM_CC_ChangeEPS ((TPM_CC)(0x00000124))
#define TPM_CC_ChangePPS ((TPM_CC)(0x00000125))
#define TPM_CC_Clear ((TPM_CC)(0x00000126))
#define TPM_CC_ClearControl ((TPM_CC)(0x00000127))
#define TPM_CC_ClockSet ((TPM_CC)(0x00000128))
#define TPM_CC_HierarchyChangeAuth ((TPM_CC)(0x00000129))
#define TPM_CC_NV_DefineSpace ((TPM_CC)(0x0000012A))
#define TPM_CC_PCR_Allocate ((TPM_CC)(0x0000012B))
#define TPM_CC_PCR_SetAuthPolicy ((TPM_CC)(0x0000012C))
#define TPM_CC_PP_Commands ((TPM_CC)(0x0000012D))
#define TPM_CC_SetPrimaryPolicy ((TPM_CC)(0x0000012E))
#define TPM_CC_FieldUpgradeStart ((TPM_CC)(0x0000012F))
#define TPM_CC_ClockRateAdjust ((TPM_CC)(0x00000130))
#define TPM_CC_CreatePrimary ((TPM_CC)(0x00000131))
#define TPM_CC_NV_GlobalWriteLock ((TPM_CC)(0x00000132))
#define TPM_CC_PP_LAST ((TPM_CC)(0x00000132))
#define TPM_CC_GetCommandAuditDigest ((TPM_CC)(0x00000133))
#define TPM_CC_NV_Increment ((TPM_CC)(0x00000134))
#define TPM_CC_NV_SetBits ((TPM_CC)(0x00000135))
#define TPM_CC_NV_Extend ((TPM_CC)(0x00000136))
#define TPM_CC_NV_Write ((TPM_CC)(0x00000137))
#define TPM_CC_NV_WriteLock ((TPM_CC)(0x00000138))
#define TPM_CC_DictionaryAttackLockReset ((TPM_CC)(0x00000139))
#define TPM_CC_DictionaryAttackParameters ((TPM_CC)(0x0000013A))
#define TPM_CC_NV_ChangeAuth ((TPM_CC)(0x0000013B))
#define TPM_CC_PCR_Event ((TPM_CC)(0x0000013C))
#define TPM_CC_PCR_Reset ((TPM_CC)(0x0000013D))
#define TPM_CC_SequenceComplete ((TPM_CC)(0x0000013E))
#define TPM_CC_SetAlgorithmSet ((TPM_CC)(0x0000013F))
#define TPM_CC_SetCommandCodeAuditStatus ((TPM_CC)(0x00000140))
#define TPM_CC_FieldUpgradeData ((TPM_CC)(0x00000141))
#define TPM_CC_IncrementalSelfTest ((TPM_CC)(0x00000142))
#define TPM_CC_SelfTest ((TPM_CC)(0x00000143))
#define TPM_CC_Startup ((TPM_CC)(0x00000144))
#define TPM_CC_Shutdown ((TPM_CC)(0x00000145))
#define TPM_CC_StirRandom ((TPM_CC)(0x00000146))
#define TPM_CC_ActivateCredential ((TPM_CC)(0x00000147))
#define TPM_CC_Certify ((TPM_CC)(0x00000148))
#define TPM_CC_PolicyNV ((TPM_CC)(0x00000149))
#define TPM_CC_CertifyCreation ((TPM_CC)(0x0000014A))
#define TPM_CC_Duplicate ((TPM_CC)(0x0000014B))
#define TPM_CC_GetTime ((TPM_CC)(0x0000014C))
#define TPM_CC_GetSessionAuditDigest ((TPM_CC)(0x0000014D))
#define TPM_CC_NV_Read ((TPM_CC)(0x0000014E))
#define TPM_CC_NV_ReadLock ((TPM_CC)(0x0000014F))
#define TPM_CC_ObjectChangeAuth ((TPM_CC)(0x00000150))
#define TPM_CC_PolicySecret ((TPM_CC)(0x00000151))
#define TPM_CC_Rewrap ((TPM_CC)(0x00000152))
#define TPM_CC_Create ((TPM_CC)(0x00000153))
#define TPM_CC_ECDH_ZGen ((TPM_CC)(0x00000154))
#define TPM_CC_HMAC ((TPM_CC)(0x00000155))
#define TPM_CC_Import ((TPM_CC)(0x00000156))
#define TPM_CC_Load ((TPM_CC)(0x00000157))
#define TPM_CC_Quote ((TPM_CC)(0x00000158))
#define TPM_CC_RSA_Decrypt ((TPM_CC)(0x00000159))
#define TPM_CC_HMAC_Start ((TPM_CC)(0x0000015B))
#define TPM_CC_SequenceUpdate ((TPM_CC)(0x0000015C))
#define TPM_CC_Sign ((TPM_CC)(0x0000015D))
#define TPM_CC_Unseal ((TPM_CC)(0x0000015E))
#define TPM_CC_PolicySigned ((TPM_CC)(0x00000160))
#define TPM_CC_ContextLoad ((TPM_CC)(0x00000161))
#define TPM_CC_ContextSave ((TPM_CC)(0x00000162))
#define TPM_CC_ECDH_KeyGen ((TPM_CC)(0x00000163))
#define TPM_CC_EncryptDecrypt ((TPM_CC)(0x00000164))
#define TPM_CC_FlushContext ((TPM_CC)(0x00000165))
#define TPM_CC_LoadExternal ((TPM_CC)(0x00000167))
#define TPM_CC_MakeCredential ((TPM_CC)(0x00000168))
#define TPM_CC_NV_ReadPublic ((TPM_CC)(0x00000169))
#define TPM_CC_PolicyAuthorize ((TPM_CC)(0x0000016A))
#define TPM_CC_PolicyAuthValue ((TPM_CC)(0x0000016B))
#define TPM_CC_PolicyCommandCode ((TPM_CC)(0x0000016C))
#define TPM_CC_PolicyCounterTimer ((TPM_CC)(0x0000016D))
#define TPM_CC_PolicyCpHash ((TPM_CC)(0x0000016E))
#define TPM_CC_PolicyLocality ((TPM_CC)(0x0000016F))
#define TPM_CC_PolicyNameHash ((TPM_CC)(0x00000170))
#define TPM_CC_PolicyOR ((TPM_CC)(0x00000171))
#define TPM_CC_PolicyTicket ((TPM_CC)(0x00000172))
#define TPM_CC_ReadPublic ((TPM_CC)(0x00000173))
#define TPM_CC_RSA_Encrypt ((TPM_CC)(0x00000174))
#define TPM_CC_StartAuthSession ((TPM_CC)(0x00000176))
#define TPM_CC_VerifySignature ((TPM_CC)(0x00000177))
#define TPM_CC_ECC_Parameters ((TPM_CC)(0x00000178))
#define TPM_CC_FirmwareRead ((TPM_CC)(0x00000179))
#define TPM_CC_GetCapability ((TPM_CC)(0x0000017A))
#define TPM_CC_GetRandom ((TPM_CC)(0x0000017B))
#define TPM_CC_GetTestResult ((TPM_CC)(0x0000017C))
#define TPM_CC_Hash ((TPM_CC)(0x0000017D))
#define TPM_CC_PCR_Read ((TPM_CC)(0x0000017E))
#define TPM_CC_PolicyPCR ((TPM_CC)(0x0000017F))
#define TPM_CC_PolicyRestart ((TPM_CC)(0x00000180))
#define TPM_CC_ReadClock ((TPM_CC)(0x00000181))
#define TPM_CC_PCR_Extend ((TPM_CC)(0x00000182))
#define TPM_CC_PCR_SetAuthValue ((TPM_CC)(0x00000183))
#define TPM_CC_NV_Certify ((TPM_CC)(0x00000184))
#define TPM_CC_EventSequenceComplete ((TPM_CC)(0x00000185))
#define TPM_CC_HashSequenceStart ((TPM_CC)(0x00000186))
#define TPM_CC_PolicyPhysicalPresence ((TPM_CC)(0x00000187))
#define TPM_CC_PolicyDuplicationSelect ((TPM_CC)(0x00000188))
#define TPM_CC_PolicyGetDigest ((TPM_CC)(0x00000189))
#define TPM_CC_TestParms ((TPM_CC)(0x0000018A))
#define TPM_CC_Commit ((TPM_CC)(0x0000018B))
#define TPM_CC_PolicyPassword ((TPM_CC)(0x0000018C))
#define TPM_CC_ZGen_2Phase ((TPM_CC)(0x0000018D))
#define TPM_CC_EC_Ephemeral ((TPM_CC)(0x0000018E))
#define TPM_CC_PolicyNvWritten ((TPM_CC)(0x0000018F))
#define TPM_CC_LAST ((TPM_CC)(0x0000018F))
#define TPM_CCE_FIRST ((TPM_CC)(0x20008001))
#define TPM_CCE_PolicyFidoSigned ((TPM_CC)(0x20008001))
#define TPM_CCE_LAST ((TPM_CC)(0x20008001))
#define TPM_RC_SUCCESS ((TPM_RC)(0x000))
#define TPM_RC_BAD_TAG ((TPM_RC)(0x01E))
#define RC_VER1 ((TPM_RC)(0x100))
#define TPM_RC_INITIALIZE ((TPM_RC)(RC_VER1 + 0x000))
#define TPM_RC_FAILURE ((TPM_RC)(RC_VER1 + 0x001))
#define TPM_RC_SEQUENCE ((TPM_RC)(RC_VER1 + 0x003))
#define TPM_RC_PRIVATE ((TPM_RC)(RC_VER1 + 0x00B))
#define TPM_RC_HMAC ((TPM_RC)(RC_VER1 + 0x019))
#define TPM_RC_DISABLED ((TPM_RC)(RC_VER1 + 0x020))
#define TPM_RC_EXCLUSIVE ((TPM_RC)(RC_VER1 + 0x021))
#define TPM_RC_AUTH_TYPE ((TPM_RC)(RC_VER1 + 0x024))
#define TPM_RC_AUTH_MISSING ((TPM_RC)(RC_VER1 + 0x025))
#define TPM_RC_POLICY ((TPM_RC)(RC_VER1 + 0x026))
#define TPM_RC_PCR ((TPM_RC)(RC_VER1 + 0x027))
#define TPM_RC_PCR_CHANGED ((TPM_RC)(RC_VER1 + 0x028))
#define TPM_RC_UPGRADE ((TPM_RC)(RC_VER1 + 0x02D))
#define TPM_RC_TOO_MANY_CONTEXTS ((TPM_RC)(RC_VER1 + 0x02E))
#define TPM_RC_AUTH_UNAVAILABLE ((TPM_RC)(RC_VER1 + 0x02F))
#define TPM_RC_REBOOT ((TPM_RC)(RC_VER1 + 0x030))
#define TPM_RC_UNBALANCED ((TPM_RC)(RC_VER1 + 0x031))
#define TPM_RC_COMMAND_SIZE ((TPM_RC)(RC_VER1 + 0x042))
#define TPM_RC_COMMAND_CODE ((TPM_RC)(RC_VER1 + 0x043))
#define TPM_RC_AUTHSIZE ((TPM_RC)(RC_VER1 + 0x044))
#define TPM_RC_AUTH_CONTEXT ((TPM_RC)(RC_VER1 + 0x045))
#define TPM_RC_NV_RANGE ((TPM_RC)(RC_VER1 + 0x046))
#define TPM_RC_NV_SIZE ((TPM_RC)(RC_VER1 + 0x047))
#define TPM_RC_NV_LOCKED ((TPM_RC)(RC_VER1 + 0x048))
#define TPM_RC_NV_AUTHORIZATION ((TPM_RC)(RC_VER1 + 0x049))
#define TPM_RC_NV_UNINITIALIZED ((TPM_RC)(RC_VER1 + 0x04A))
#define TPM_RC_NV_SPACE ((TPM_RC)(RC_VER1 + 0x04B))
#define TPM_RC_NV_DEFINED ((TPM_RC)(RC_VER1 + 0x04C))
#define TPM_RC_BAD_CONTEXT ((TPM_RC)(RC_VER1 + 0x050))
#define TPM_RC_CPHASH ((TPM_RC)(RC_VER1 + 0x051))
#define TPM_RC_PARENT ((TPM_RC)(RC_VER1 + 0x052))
#define TPM_RC_NEEDS_TEST ((TPM_RC)(RC_VER1 + 0x053))
#define TPM_RC_NO_RESULT ((TPM_RC)(RC_VER1 + 0x054))
#define TPM_RC_SENSITIVE ((TPM_RC)(RC_VER1 + 0x055))
#define RC_MAX_FM0 ((TPM_RC)(RC_VER1 + 0x07F))
#define RC_FMT1 ((TPM_RC)(0x080))
#define TPM_RC_ASYMMETRIC ((TPM_RC)(RC_FMT1 + 0x001))
#define TPM_RC_ATTRIBUTES ((TPM_RC)(RC_FMT1 + 0x002))
#define TPM_RC_HASH ((TPM_RC)(RC_FMT1 + 0x003))
#define TPM_RC_VALUE ((TPM_RC)(RC_FMT1 + 0x004))
#define TPM_RC_HIERARCHY ((TPM_RC)(RC_FMT1 + 0x005))
#define TPM_RC_KEY_SIZE ((TPM_RC)(RC_FMT1 + 0x007))
#define TPM_RC_MGF ((TPM_RC)(RC_FMT1 + 0x008))
#define TPM_RC_MODE ((TPM_RC)(RC_FMT1 + 0x009))
#define TPM_RC_TYPE ((TPM_RC)(RC_FMT1 + 0x00A))
#define TPM_RC_HANDLE ((TPM_RC)(RC_FMT1 + 0x00B))
#define TPM_RC_KDF ((TPM_RC)(RC_FMT1 + 0x00C))
#define TPM_RC_RANGE ((TPM_RC)(RC_FMT1 + 0x00D))
#define TPM_RC_AUTH_FAIL ((TPM_RC)(RC_FMT1 + 0x00E))
#define TPM_RC_NONCE ((TPM_RC)(RC_FMT1 + 0x00F))
#define TPM_RC_PP ((TPM_RC)(RC_FMT1 + 0x010))
#define TPM_RC_SCHEME ((TPM_RC)(RC_FMT1 + 0x012))
#define TPM_RC_SIZE ((TPM_RC)(RC_FMT1 + 0x015))
#define TPM_RC_SYMMETRIC ((TPM_RC)(RC_FMT1 + 0x016))
#define TPM_RC_TAG ((TPM_RC)(RC_FMT1 + 0x017))
#define TPM_RC_SELECTOR ((TPM_RC)(RC_FMT1 + 0x018))
#define TPM_RC_INSUFFICIENT ((TPM_RC)(RC_FMT1 + 0x01A))
#define TPM_RC_SIGNATURE ((TPM_RC)(RC_FMT1 + 0x01B))
#define TPM_RC_KEY ((TPM_RC)(RC_FMT1 + 0x01C))
#define TPM_RC_POLICY_FAIL ((TPM_RC)(RC_FMT1 + 0x01D))
#define TPM_RC_INTEGRITY ((TPM_RC)(RC_FMT1 + 0x01F))
#define TPM_RC_TICKET ((TPM_RC)(RC_FMT1 + 0x020))
#define TPM_RC_RESERVED_BITS ((TPM_RC)(RC_FMT1 + 0x021))
#define TPM_RC_BAD_AUTH ((TPM_RC)(RC_FMT1 + 0x022))
#define TPM_RC_EXPIRED ((TPM_RC)(RC_FMT1 + 0x023))
#define TPM_RC_POLICY_CC ((TPM_RC)(RC_FMT1 + 0x024))
#define TPM_RC_BINDING ((TPM_RC)(RC_FMT1 + 0x025))
#define TPM_RC_CURVE ((TPM_RC)(RC_FMT1 + 0x026))
#define TPM_RC_ECC_POINT ((TPM_RC)(RC_FMT1 + 0x027))
#define RC_WARN ((TPM_RC)(0x900))
#define TPM_RC_CONTEXT_GAP ((TPM_RC)(RC_WARN + 0x001))
#define TPM_RC_OBJECT_MEMORY ((TPM_RC)(RC_WARN + 0x002))
#define TPM_RC_SESSION_MEMORY ((TPM_RC)(RC_WARN + 0x003))
#define TPM_RC_MEMORY ((TPM_RC)(RC_WARN + 0x004))
#define TPM_RC_SESSION_HANDLES ((TPM_RC)(RC_WARN + 0x005))
#define TPM_RC_OBJECT_HANDLES ((TPM_RC)(RC_WARN + 0x006))
#define TPM_RC_LOCALITY ((TPM_RC)(RC_WARN + 0x007))
#define TPM_RC_YIELDED ((TPM_RC)(RC_WARN + 0x008))
#define TPM_RC_CANCELED ((TPM_RC)(RC_WARN + 0x009))
#define TPM_RC_TESTING ((TPM_RC)(RC_WARN + 0x00A))
#define TPM_RC_REFERENCE_H0 ((TPM_RC)(RC_WARN + 0x010))
#define TPM_RC_REFERENCE_H1 ((TPM_RC)(RC_WARN + 0x011))
#define TPM_RC_REFERENCE_H2 ((TPM_RC)(RC_WARN + 0x012))
#define TPM_RC_REFERENCE_H3 ((TPM_RC)(RC_WARN + 0x013))
#define TPM_RC_REFERENCE_H4 ((TPM_RC)(RC_WARN + 0x014))
#define TPM_RC_REFERENCE_H5 ((TPM_RC)(RC_WARN + 0x015))
#define TPM_RC_REFERENCE_H6 ((TPM_RC)(RC_WARN + 0x016))
#define TPM_RC_REFERENCE_S0 ((TPM_RC)(RC_WARN + 0x018))
#define TPM_RC_REFERENCE_S1 ((TPM_RC)(RC_WARN + 0x019))
#define TPM_RC_REFERENCE_S2 ((TPM_RC)(RC_WARN + 0x01A))
#define TPM_RC_REFERENCE_S3 ((TPM_RC)(RC_WARN + 0x01B))
#define TPM_RC_REFERENCE_S4 ((TPM_RC)(RC_WARN + 0x01C))
#define TPM_RC_REFERENCE_S5 ((TPM_RC)(RC_WARN + 0x01D))
#define TPM_RC_REFERENCE_S6 ((TPM_RC)(RC_WARN + 0x01E))
#define TPM_RC_NV_RATE ((TPM_RC)(RC_WARN + 0x020))
#define TPM_RC_LOCKOUT ((TPM_RC)(RC_WARN + 0x021))
#define TPM_RC_RETRY ((TPM_RC)(RC_WARN + 0x022))
#define TPM_RC_NV_UNAVAILABLE ((TPM_RC)(RC_WARN + 0x023))
#define TPM_RC_NOT_USED ((TPM_RC)(RC_WARN + 0x7F))
#define TPM_RC_H ((TPM_RC)(0x000))
#define TPM_RC_P ((TPM_RC)(0x040))
#define TPM_RC_S ((TPM_RC)(0x800))
#define TPM_RC_1 ((TPM_RC)(0x100))
#define TPM_RC_2 ((TPM_RC)(0x200))
#define TPM_RC_3 ((TPM_RC)(0x300))
#define TPM_RC_4 ((TPM_RC)(0x400))
#define TPM_RC_5 ((TPM_RC)(0x500))
#define TPM_RC_6 ((TPM_RC)(0x600))
#define TPM_RC_7 ((TPM_RC)(0x700))
#define TPM_RC_8 ((TPM_RC)(0x800))
#define TPM_RC_9 ((TPM_RC)(0x900))
#define TPM_RC_A ((TPM_RC)(0xA00))
#define TPM_RC_B ((TPM_RC)(0xB00))
#define TPM_RC_C ((TPM_RC)(0xC00))
#define TPM_RC_D ((TPM_RC)(0xD00))
#define TPM_RC_E ((TPM_RC)(0xE00))
#define TPM_RC_F ((TPM_RC)(0xF00))
#define TPM_RC_N_MASK ((TPM_RC)(0xF00))
#define TPM_CLOCK_COARSE_SLOWER ((TPM_CLOCK_ADJUST)(-3))
#define TPM_CLOCK_MEDIUM_SLOWER ((TPM_CLOCK_ADJUST)(-2))
#define TPM_CLOCK_FINE_SLOWER ((TPM_CLOCK_ADJUST)(-1))
#define TPM_CLOCK_NO_CHANGE ((TPM_CLOCK_ADJUST)(0))
#define TPM_CLOCK_FINE_FASTER ((TPM_CLOCK_ADJUST)(1))
#define TPM_CLOCK_MEDIUM_FASTER ((TPM_CLOCK_ADJUST)(2))
#define TPM_CLOCK_COARSE_FASTER ((TPM_CLOCK_ADJUST)(3))
#define TPM_EO_EQ ((TPM_EO)(0x0000))
#define TPM_EO_NEQ ((TPM_EO)(0x0001))
#define TPM_EO_SIGNED_GT ((TPM_EO)(0x0002))
#define TPM_EO_UNSIGNED_GT ((TPM_EO)(0x0003))
#define TPM_EO_SIGNED_LT ((TPM_EO)(0x0004))
#define TPM_EO_UNSIGNED_LT ((TPM_EO)(0x0005))
#define TPM_EO_SIGNED_GE ((TPM_EO)(0x0006))
#define TPM_EO_UNSIGNED_GE ((TPM_EO)(0x0007))
#define TPM_EO_SIGNED_LE ((TPM_EO)(0x0008))
#define TPM_EO_UNSIGNED_LE ((TPM_EO)(0x0009))
#define TPM_EO_BITSET ((TPM_EO)(0x000A))
#define TPM_EO_BITCLEAR ((TPM_EO)(0x000B))
#define TPM_ST_RSP_COMMAND ((TPM_ST)(0x00C4))
#define TPM_ST_NULL ((TPM_ST)(0X8000))
#define TPM_ST_NO_SESSIONS ((TPM_ST)(0x8001))
#define TPM_ST_SESSIONS ((TPM_ST)(0x8002))
#define TPM_ST_ATTEST_NV ((TPM_ST)(0x8014))
#define TPM_ST_ATTEST_COMMAND_AUDIT ((TPM_ST)(0x8015))
#define TPM_ST_ATTEST_SESSION_AUDIT ((TPM_ST)(0x8016))
#define TPM_ST_ATTEST_CERTIFY ((TPM_ST)(0x8017))
#define TPM_ST_ATTEST_QUOTE ((TPM_ST)(0x8018))
#define TPM_ST_ATTEST_TIME ((TPM_ST)(0x8019))
#define TPM_ST_ATTEST_CREATION ((TPM_ST)(0x801A))
#define TPM_ST_CREATION ((TPM_ST)(0x8021))
#define TPM_ST_VERIFIED ((TPM_ST)(0x8022))
#define TPM_ST_AUTH_SECRET ((TPM_ST)(0x8023))
#define TPM_ST_HASHCHECK ((TPM_ST)(0x8024))
#define TPM_ST_AUTH_SIGNED ((TPM_ST)(0x8025))
#define TPM_ST_FU_MANIFEST ((TPM_ST)(0x8029))
#define TPM_SU_CLEAR ((TPM_SU)(0x0000))
#define TPM_SU_STATE ((TPM_SU)(0x0001))
#define TPM_SE_HMAC ((TPM_SE)(0x00))
#define TPM_SE_POLICY ((TPM_SE)(0x01))
#define TPM_SE_TRIAL ((TPM_SE)(0x03))
#define TPM_CAP_FIRST ((TPM_CAP)(0x00000000))
#define TPM_CAP_ALGS ((TPM_CAP)(0x00000000))
#define TPM_CAP_HANDLES ((TPM_CAP)(0x00000001))
#define TPM_CAP_COMMANDS ((TPM_CAP)(0x00000002))
#define TPM_CAP_PP_COMMANDS ((TPM_CAP)(0x00000003))
#define TPM_CAP_AUDIT_COMMANDS ((TPM_CAP)(0x00000004))
#define TPM_CAP_PCRS ((TPM_CAP)(0x00000005))
#define TPM_CAP_TPM_PROPERTIES ((TPM_CAP)(0x00000006))
#define TPM_CAP_PCR_PROPERTIES ((TPM_CAP)(0x00000007))
#define TPM_CAP_ECC_CURVES ((TPM_CAP)(0x00000008))
#define TPM_CAP_LAST ((TPM_CAP)(0x00000008))
#define TPM_CAP_VENDOR_PROPERTY ((TPM_CAP)(0x00000100))
#define TPM_PT_NONE ((TPM_PT)(0x00000000))
#define PT_GROUP ((TPM_PT)(0x00000100))
#define PT_FIXED ((TPM_PT)(PT_GROUP * 1))
#define TPM_PT_FAMILY_INDICATOR ((TPM_PT)(PT_FIXED + 0))
#define TPM_PT_LEVEL ((TPM_PT)(PT_FIXED + 1))
#define TPM_PT_REVISION ((TPM_PT)(PT_FIXED + 2))
#define TPM_PT_DAY_OF_YEAR ((TPM_PT)(PT_FIXED + 3))
#define TPM_PT_YEAR ((TPM_PT)(PT_FIXED + 4))
#define TPM_PT_MANUFACTURER ((TPM_PT)(PT_FIXED + 5))
#define TPM_PT_VENDOR_STRING_1 ((TPM_PT)(PT_FIXED + 6))
#define TPM_PT_VENDOR_STRING_2 ((TPM_PT)(PT_FIXED + 7))
#define TPM_PT_VENDOR_STRING_3 ((TPM_PT)(PT_FIXED + 8))
#define TPM_PT_VENDOR_STRING_4 ((TPM_PT)(PT_FIXED + 9))
#define TPM_PT_VENDOR_TPM_TYPE ((TPM_PT)(PT_FIXED + 10))
#define TPM_PT_FIRMWARE_VERSION_1 ((TPM_PT)(PT_FIXED + 11))
#define TPM_PT_FIRMWARE_VERSION_2 ((TPM_PT)(PT_FIXED + 12))
#define TPM_PT_INPUT_BUFFER ((TPM_PT)(PT_FIXED + 13))
#define TPM_PT_HR_TRANSIENT_MIN ((TPM_PT)(PT_FIXED + 14))
#define TPM_PT_HR_PERSISTENT_MIN ((TPM_PT)(PT_FIXED + 15))
#define TPM_PT_HR_LOADED_MIN ((TPM_PT)(PT_FIXED + 16))
#define TPM_PT_ACTIVE_SESSIONS_MAX ((TPM_PT)(PT_FIXED + 17))
#define TPM_PT_PCR_COUNT ((TPM_PT)(PT_FIXED + 18))
#define TPM_PT_PCR_SELECT_MIN ((TPM_PT)(PT_FIXED + 19))
#define TPM_PT_CONTEXT_GAP_MAX ((TPM_PT)(PT_FIXED + 20))
#define TPM_PT_NV_COUNTERS_MAX ((TPM_PT)(PT_FIXED + 22))
#define TPM_PT_NV_INDEX_MAX ((TPM_PT)(PT_FIXED + 23))
#define TPM_PT_MEMORY ((TPM_PT)(PT_FIXED + 24))
#define TPM_PT_CLOCK_UPDATE ((TPM_PT)(PT_FIXED + 25))
#define TPM_PT_CONTEXT_HASH ((TPM_PT)(PT_FIXED + 26))
#define TPM_PT_CONTEXT_SYM ((TPM_PT)(PT_FIXED + 27))
#define TPM_PT_CONTEXT_SYM_SIZE ((TPM_PT)(PT_FIXED + 28))
#define TPM_PT_ORDERLY_COUNT ((TPM_PT)(PT_FIXED + 29))
#define TPM_PT_MAX_COMMAND_SIZE ((TPM_PT)(PT_FIXED + 30))
#define TPM_PT_MAX_RESPONSE_SIZE ((TPM_PT)(PT_FIXED + 31))
#define TPM_PT_MAX_DIGEST ((TPM_PT)(PT_FIXED + 32))
#define TPM_PT_MAX_OBJECT_CONTEXT ((TPM_PT)(PT_FIXED + 33))
#define TPM_PT_MAX_SESSION_CONTEXT ((TPM_PT)(PT_FIXED + 34))
#define TPM_PT_PS_FAMILY_INDICATOR ((TPM_PT)(PT_FIXED + 35))
#define TPM_PT_PS_LEVEL ((TPM_PT)(PT_FIXED + 36))
#define TPM_PT_PS_REVISION ((TPM_PT)(PT_FIXED + 37))
#define TPM_PT_PS_DAY_OF_YEAR ((TPM_PT)(PT_FIXED + 38))
#define TPM_PT_PS_YEAR ((TPM_PT)(PT_FIXED + 39))
#define TPM_PT_SPLIT_MAX ((TPM_PT)(PT_FIXED + 40))
#define TPM_PT_TOTAL_COMMANDS ((TPM_PT)(PT_FIXED + 41))
#define TPM_PT_LIBRARY_COMMANDS ((TPM_PT)(PT_FIXED + 42))
#define TPM_PT_VENDOR_COMMANDS ((TPM_PT)(PT_FIXED + 43))
#define TPM_PT_NV_BUFFER_MAX ((TPM_PT)(PT_FIXED + 44))
#define PT_VAR ((TPM_PT)(PT_GROUP * 2))
#define TPM_PT_PERMANENT ((TPM_PT)(PT_VAR + 0))
#define TPM_PT_STARTUP_CLEAR ((TPM_PT)(PT_VAR + 1))
#define TPM_PT_HR_NV_INDEX ((TPM_PT)(PT_VAR + 2))
#define TPM_PT_HR_LOADED ((TPM_PT)(PT_VAR + 3))
#define TPM_PT_HR_LOADED_AVAIL ((TPM_PT)(PT_VAR + 4))
#define TPM_PT_HR_ACTIVE ((TPM_PT)(PT_VAR + 5))
#define TPM_PT_HR_ACTIVE_AVAIL ((TPM_PT)(PT_VAR + 6))
#define TPM_PT_HR_TRANSIENT_AVAIL ((TPM_PT)(PT_VAR + 7))
#define TPM_PT_HR_PERSISTENT ((TPM_PT)(PT_VAR + 8))
#define TPM_PT_HR_PERSISTENT_AVAIL ((TPM_PT)(PT_VAR + 9))
#define TPM_PT_NV_COUNTERS ((TPM_PT)(PT_VAR + 10))
#define TPM_PT_NV_COUNTERS_AVAIL ((TPM_PT)(PT_VAR + 11))
#define TPM_PT_ALGORITHM_SET ((TPM_PT)(PT_VAR + 12))
#define TPM_PT_LOADED_CURVES ((TPM_PT)(PT_VAR + 13))
#define TPM_PT_LOCKOUT_COUNTER ((TPM_PT)(PT_VAR + 14))
#define TPM_PT_MAX_AUTH_FAIL ((TPM_PT)(PT_VAR + 15))
#define TPM_PT_LOCKOUT_INTERVAL ((TPM_PT)(PT_VAR + 16))
#define TPM_PT_LOCKOUT_RECOVERY ((TPM_PT)(PT_VAR + 17))
#define TPM_PT_NV_WRITE_RECOVERY ((TPM_PT)(PT_VAR + 18))
#define TPM_PT_AUDIT_COUNTER_0 ((TPM_PT)(PT_VAR + 19))
#define TPM_PT_AUDIT_COUNTER_1 ((TPM_PT)(PT_VAR + 20))
#define TPM_PT_PCR_FIRST ((TPM_PT_PCR)(0x00000000))
#define TPM_PT_PCR_SAVE ((TPM_PT_PCR)(0x00000000))
#define TPM_PT_PCR_EXTEND_L0 ((TPM_PT_PCR)(0x00000001))
#define TPM_PT_PCR_RESET_L0 ((TPM_PT_PCR)(0x00000002))
#define TPM_PT_PCR_EXTEND_L1 ((TPM_PT_PCR)(0x00000003))
#define TPM_PT_PCR_RESET_L1 ((TPM_PT_PCR)(0x00000004))
#define TPM_PT_PCR_EXTEND_L2 ((TPM_PT_PCR)(0x00000005))
#define TPM_PT_PCR_RESET_L2 ((TPM_PT_PCR)(0x00000006))
#define TPM_PT_PCR_EXTEND_L3 ((TPM_PT_PCR)(0x00000007))
#define TPM_PT_PCR_RESET_L3 ((TPM_PT_PCR)(0x00000008))
#define TPM_PT_PCR_EXTEND_L4 ((TPM_PT_PCR)(0x00000009))
#define TPM_PT_PCR_RESET_L4 ((TPM_PT_PCR)(0x0000000A))
#define TPM_PT_PCR_NO_INCREMENT ((TPM_PT_PCR)(0x00000011))
#define TPM_PT_PCR_DRTM_RESET ((TPM_PT_PCR)(0x00000012))
#define TPM_PT_PCR_POLICY ((TPM_PT_PCR)(0x00000013))
#define TPM_PT_PCR_AUTH ((TPM_PT_PCR)(0x00000014))
#define TPM_PT_PCR_LAST ((TPM_PT_PCR)(0x00000014))
#define TPM_PS_MAIN ((TPM_PS)(0x00000000))
#define TPM_PS_PC ((TPM_PS)(0x00000001))
#define TPM_PS_PDA ((TPM_PS)(0x00000002))
#define TPM_PS_CELL_PHONE ((TPM_PS)(0x00000003))
#define TPM_PS_SERVER ((TPM_PS)(0x00000004))
#define TPM_PS_PERIPHERAL ((TPM_PS)(0x00000005))
#define TPM_PS_TSS ((TPM_PS)(0x00000006))
#define TPM_PS_STORAGE ((TPM_PS)(0x00000007))
#define TPM_PS_AUTHENTICATION ((TPM_PS)(0x00000008))
#define TPM_PS_EMBEDDED ((TPM_PS)(0x00000009))
#define TPM_PS_HARDCOPY ((TPM_PS)(0x0000000A))
#define TPM_PS_INFRASTRUCTURE ((TPM_PS)(0x0000000B))
#define TPM_PS_VIRTUALIZATION ((TPM_PS)(0x0000000C))
#define TPM_PS_TNC ((TPM_PS)(0x0000000D))
#define TPM_PS_MULTI_TENANT ((TPM_PS)(0x0000000E))
#define TPM_PS_TC ((TPM_PS)(0x0000000F))
#define TPM_HT_PCR ((TPM_HT)(0x00))
#define TPM_HT_NV_INDEX ((TPM_HT)(0x01))
#define TPM_HT_HMAC_SESSION ((TPM_HT)(0x02))
#define TPM_HT_LOADED_SESSION ((TPM_HT)(0x02))
#define TPM_HT_POLICY_SESSION ((TPM_HT)(0x03))
#define TPM_HT_ACTIVE_SESSION ((TPM_HT)(0x03))
#define TPM_HT_PERMANENT ((TPM_HT)(0x40))
#define TPM_HT_TRANSIENT ((TPM_HT)(0x80))
#define TPM_HT_PERSISTENT ((TPM_HT)(0x81))
#define TPM_RH_FIRST ((TPM_RH)(0x40000000))
#define TPM_RH_SRK ((TPM_RH)(0x40000000))
#define TPM_RH_OWNER ((TPM_RH)(0x40000001))
#define TPM_RH_REVOKE ((TPM_RH)(0x40000002))
#define TPM_RH_TRANSPORT ((TPM_RH)(0x40000003))
#define TPM_RH_OPERATOR ((TPM_RH)(0x40000004))
#define TPM_RH_ADMIN ((TPM_RH)(0x40000005))
#define TPM_RH_EK ((TPM_RH)(0x40000006))
#define TPM_RH_NULL ((TPM_RH)(0x40000007))
#define TPM_RH_UNASSIGNED ((TPM_RH)(0x40000008))
#define TPM_RS_PW ((TPM_RH)(0x40000009))
#define TPM_RH_LOCKOUT ((TPM_RH)(0x4000000A))
#define TPM_RH_ENDORSEMENT ((TPM_RH)(0x4000000B))
#define TPM_RH_PLATFORM ((TPM_RH)(0x4000000C))
#define TPM_RH_PLATFORM_NV ((TPM_RH)(0x4000000D))
#define TPM_RH_LAST ((TPM_RH)(0x4000000D))
#define HR_HANDLE_MASK ((TPM_HC)(0x00FFFFFF))
#define HR_RANGE_MASK ((TPM_HC)(0xFF000000))
#define HR_SHIFT ((TPM_HC)(24))
#define HR_PCR (((TPM_HC)(TPM_HT_PCR)) << HR_SHIFT)
#define HR_HMAC_SESSION (((TPM_HC)(TPM_HT_HMAC_SESSION)) << HR_SHIFT)
#define HR_POLICY_SESSION (((TPM_HC)(TPM_HT_POLICY_SESSION)) << HR_SHIFT)
#define HR_TRANSIENT (((TPM_HC)(TPM_HT_TRANSIENT)) << HR_SHIFT)
#define HR_PERSISTENT (((TPM_HC)(TPM_HT_PERSISTENT)) << HR_SHIFT)
#define HR_NV_INDEX (((TPM_HC)(TPM_HT_NV_INDEX)) << HR_SHIFT)
#define HR_PERMANENT (((TPM_HC)(TPM_HT_PERMANENT)) << HR_SHIFT)
#define PCR_FIRST ((TPM_HC)((HR_PCR + 0)))
#define PCR_LAST ((TPM_HC)((PCR_FIRST + IMPLEMENTATION_PCR - 1)))
#define HMAC_SESSION_FIRST ((TPM_HC)((HR_HMAC_SESSION + 0)))
#define HMAC_SESSION_LAST ((TPM_HC)(HMAC_SESSION_FIRST + MAX_ACTIVE_SESSIONS - 1))
#define LOADED_SESSION_LAST ((TPM_HC)(HMAC_SESSION_LAST))
#define POLICY_SESSION_FIRST ((TPM_HC)((HR_POLICY_SESSION + 0)))
#define POLICY_SESSION_LAST ((TPM_HC)(POLICY_SESSION_FIRST + MAX_ACTIVE_SESSIONS - 1))
#define TRANSIENT_FIRST ((TPM_HC)((HR_TRANSIENT + 0)))
#define ACTIVE_SESSION_FIRST ((TPM_HC)(POLICY_SESSION_FIRST))
#define ACTIVE_SESSION_LAST ((TPM_HC)(POLICY_SESSION_LAST))
#define TRANSIENT_LAST ((TPM_HC)((TRANSIENT_FIRST + MAX_LOADED_OBJECTS - 1)))
#define PERSISTENT_FIRST ((TPM_HC)((HR_PERSISTENT + 0)))
#define PERSISTENT_LAST ((TPM_HC)((PERSISTENT_FIRST + 0x00FFFFFF)))
#define PLATFORM_PERSISTENT ((TPM_HC)((PERSISTENT_FIRST + 0x00800000)))
#define NV_INDEX_FIRST ((TPM_HC)((HR_NV_INDEX + 0)))
#define NV_INDEX_LAST ((TPM_HC)((NV_INDEX_FIRST + 0x00FFFFFF)))
#define PERMANENT_FIRST ((TPM_HC)(TPM_RH_FIRST))
#define PERMANENT_LAST ((TPM_HC)(TPM_RH_LAST))

typedef struct TPMS_ALGORITHM_DESCRIPTION {
  TPM_ALG_ID alg;
  TPMA_ALGORITHM attributes;
} TPMS_ALGORITHM_DESCRIPTION;

typedef union TPMU_HA {
  BYTE sha1[SHA1_DIGEST_SIZE];
  BYTE sha256[SHA256_DIGEST_SIZE];
  BYTE sm3_256[SM3_256_DIGEST_SIZE];
  BYTE sha384[SHA384_DIGEST_SIZE];
  BYTE sha512[SHA512_DIGEST_SIZE];
} TPMU_HA;

typedef struct TPMT_HA {
  TPMI_ALG_HASH hash_alg;
  TPMU_HA digest;
} TPMT_HA;

typedef struct TPM2B_DIGEST {
  UINT16 size;
  BYTE buffer[sizeof(TPMU_HA)];
} TPM2B_DIGEST;

typedef struct TPM2B_DATA {
  UINT16 size;
  BYTE buffer[sizeof(TPMT_HA)];
} TPM2B_DATA;

typedef struct TPM2B_EVENT {
  UINT16 size;
  BYTE buffer[1024];
} TPM2B_EVENT;

typedef struct TPM2B_MAX_BUFFER {
  UINT16 size;
  BYTE buffer[MAX_DIGEST_BUFFER];
} TPM2B_MAX_BUFFER;

typedef struct TPM2B_MAX_NV_BUFFER {
  UINT16 size;
  BYTE buffer[MAX_NV_BUFFER_SIZE];
} TPM2B_MAX_NV_BUFFER;

typedef struct TPM2B_TIMEOUT {
  UINT16 size;
  BYTE buffer[sizeof(UINT64)];
} TPM2B_TIMEOUT;

typedef struct TPM2B_IV {
  UINT16 size;
  BYTE buffer[MAX_SYM_BLOCK_SIZE];
} TPM2B_IV;

typedef union TPMU_NAME {
  TPMT_HA digest;
  TPM_HANDLE handle;
} TPMU_NAME;

typedef struct TPM2B_NAME {
  UINT16 size;
  BYTE name[sizeof(TPMU_NAME)];
} TPM2B_NAME;

typedef struct TPMS_PCR_SELECT {
  UINT8 sizeof_select;
  BYTE pcr_select[PCR_SELECT_MAX];
} TPMS_PCR_SELECT;

typedef struct TPMS_PCR_SELECTION {
  TPMI_ALG_HASH hash;
  UINT8 sizeof_select;
  BYTE pcr_select[PCR_SELECT_MAX];
} TPMS_PCR_SELECTION;

typedef struct TPMT_TK_CREATION {
  TPM_ST tag;
  TPMI_RH_HIERARCHY hierarchy;
  TPM2B_DIGEST digest;
} TPMT_TK_CREATION;

typedef struct TPMT_TK_VERIFIED {
  TPM_ST tag;
  TPMI_RH_HIERARCHY hierarchy;
  TPM2B_DIGEST digest;
} TPMT_TK_VERIFIED;

typedef struct TPMT_TK_AUTH {
  TPM_ST tag;
  TPMI_RH_HIERARCHY hierarchy;
  TPM2B_DIGEST digest;
} TPMT_TK_AUTH;

typedef struct TPMT_TK_HASHCHECK {
  TPM_ST tag;
  TPMI_RH_HIERARCHY hierarchy;
  TPM2B_DIGEST digest;
} TPMT_TK_HASHCHECK;

typedef struct TPMS_ALG_PROPERTY {
  TPM_ALG_ID alg;
  TPMA_ALGORITHM alg_properties;
} TPMS_ALG_PROPERTY;

typedef struct TPMS_TAGGED_PROPERTY {
  TPM_PT property;
  UINT32 value;
} TPMS_TAGGED_PROPERTY;

typedef struct TPMS_TAGGED_PCR_SELECT {
  TPM_PT tag;
  UINT8 sizeof_select;
  BYTE pcr_select[PCR_SELECT_MAX];
} TPMS_TAGGED_PCR_SELECT;

typedef struct TPML_CC {
  UINT32 count;
  TPM_CC command_codes[MAX_CAP_CC_ALL];
} TPML_CC;

typedef struct TPML_CCA {
  UINT32 count;
  TPMA_CC command_attributes[MAX_CAP_CC_ALL];
} TPML_CCA;

typedef struct TPML_ALG {
  UINT32 count;
  TPM_ALG_ID algorithms[MAX_ALG_LIST_SIZE];
} TPML_ALG;

typedef struct TPML_HANDLE {
  UINT32 count;
  TPM_HANDLE handle[MAX_CAP_HANDLES];
} TPML_HANDLE;

typedef struct TPML_DIGEST {
  UINT32 count;
  TPM2B_DIGEST digests[8];
} TPML_DIGEST;

typedef struct TPML_DIGEST_VALUES {
  UINT32 count;
  TPMT_HA digests[HASH_COUNT];
} TPML_DIGEST_VALUES;

typedef struct TPM2B_DIGEST_VALUES {
  UINT16 size;
  BYTE buffer[sizeof(TPML_DIGEST_VALUES)];
} TPM2B_DIGEST_VALUES;

typedef struct TPML_PCR_SELECTION {
  UINT32 count;
  TPMS_PCR_SELECTION pcr_selections[HASH_COUNT];
} TPML_PCR_SELECTION;

typedef struct TPML_ALG_PROPERTY {
  UINT32 count;
  TPMS_ALG_PROPERTY alg_properties[MAX_CAP_ALGS];
} TPML_ALG_PROPERTY;

typedef struct TPML_TAGGED_TPM_PROPERTY {
  UINT32 count;
  TPMS_TAGGED_PROPERTY tpm_property[MAX_TPM_PROPERTIES];
} TPML_TAGGED_TPM_PROPERTY;

typedef struct TPML_TAGGED_PCR_PROPERTY {
  UINT32 count;
  TPMS_TAGGED_PCR_SELECT pcr_property[MAX_PCR_PROPERTIES];
} TPML_TAGGED_PCR_PROPERTY;

typedef struct TPML_ECC_CURVE {
  UINT32 count;
  TPM_ECC_CURVE ecc_curves[MAX_ECC_CURVES];
} TPML_ECC_CURVE;

typedef union TPMU_CAPABILITIES {
  TPML_ALG_PROPERTY algorithms;
  TPML_HANDLE handles;
  TPML_CCA command;
  TPML_CC pp_commands;
  TPML_CC audit_commands;
  TPML_PCR_SELECTION assigned_pcr;
  TPML_TAGGED_TPM_PROPERTY tpm_properties;
  TPML_TAGGED_PCR_PROPERTY pcr_properties;
  TPML_ECC_CURVE ecc_curves;
} TPMU_CAPABILITIES;

typedef struct TPMS_CAPABILITY_DATA {
  TPM_CAP capability;
  TPMU_CAPABILITIES data;
} TPMS_CAPABILITY_DATA;

typedef struct TPMS_CLOCK_INFO {
  UINT64 clock;
  UINT32 reset_count;
  UINT32 restart_count;
  TPMI_YES_NO safe;
} TPMS_CLOCK_INFO;

typedef struct TPMS_TIME_INFO {
  UINT64 time;
  TPMS_CLOCK_INFO clock_info;
} TPMS_TIME_INFO;

typedef struct TPMS_TIME_ATTEST_INFO {
  TPMS_TIME_INFO time;
  UINT64 firmware_version;
} TPMS_TIME_ATTEST_INFO;

typedef struct TPMS_CERTIFY_INFO {
  TPM2B_NAME name;
  TPM2B_NAME qualified_name;
} TPMS_CERTIFY_INFO;

typedef struct TPMS_QUOTE_INFO {
  TPML_PCR_SELECTION pcr_select;
  TPM2B_DIGEST pcr_digest;
} TPMS_QUOTE_INFO;

typedef struct TPMS_COMMAND_AUDIT_INFO {
  UINT64 audit_counter;
  TPM_ALG_ID digest_alg;
  TPM2B_DIGEST audit_digest;
  TPM2B_DIGEST command_digest;
} TPMS_COMMAND_AUDIT_INFO;

typedef struct TPMS_SESSION_AUDIT_INFO {
  TPMI_YES_NO exclusive_session;
  TPM2B_DIGEST session_digest;
} TPMS_SESSION_AUDIT_INFO;

typedef struct TPMS_CREATION_INFO {
  TPM2B_NAME object_name;
  TPM2B_DIGEST creation_hash;
} TPMS_CREATION_INFO;

typedef struct TPMS_NV_CERTIFY_INFO {
  TPM2B_NAME index_name;
  UINT16 offset;
  TPM2B_MAX_NV_BUFFER nv_contents;
} TPMS_NV_CERTIFY_INFO;

typedef union TPMU_ATTEST {
  TPMS_CERTIFY_INFO certify;
  TPMS_CREATION_INFO creation;
  TPMS_QUOTE_INFO quote;
  TPMS_COMMAND_AUDIT_INFO command_audit;
  TPMS_SESSION_AUDIT_INFO session_audit;
  TPMS_TIME_ATTEST_INFO time;
  TPMS_NV_CERTIFY_INFO nv;
} TPMU_ATTEST;

typedef struct TPMS_ATTEST {
  TPM_GENERATED magic;
  TPMI_ST_ATTEST type;
  TPM2B_NAME qualified_signer;
  TPM2B_DATA extra_data;
  TPMS_CLOCK_INFO clock_info;
  UINT64 firmware_version;
  TPMU_ATTEST attested;
} TPMS_ATTEST;

typedef struct TPM2B_ATTEST {
  UINT16 size;
  BYTE attestation_data[sizeof(TPMS_ATTEST)];
} TPM2B_ATTEST;

typedef struct TPMS_AUTH_COMMAND {
  TPMI_SH_AUTH_SESSION session_handle;
  TPM2B_NONCE nonce;
  TPMA_SESSION session_attributes;
  TPM2B_AUTH hmac;
} TPMS_AUTH_COMMAND;

typedef struct TPMS_AUTH_RESPONSE {
  TPM2B_NONCE nonce;
  TPMA_SESSION session_attributes;
  TPM2B_AUTH hmac;
} TPMS_AUTH_RESPONSE;

typedef union TPMU_SYM_KEY_BITS {
  TPMI_AES_KEY_BITS aes;
  TPMI_SM4_KEY_BITS sm4;
  TPM_KEY_BITS sym;
  TPMI_ALG_HASH xor_;
} TPMU_SYM_KEY_BITS;

typedef union TPMU_SYM_MODE {
  TPMI_ALG_SYM_MODE aes;
  TPMI_ALG_SYM_MODE sm4;
  TPMI_ALG_SYM_MODE sym;
} TPMU_SYM_MODE;

#ifdef __cplusplus
typedef char TPMU_SYM_DETAILS[0];
static_assert(sizeof(TPMU_SYM_DETAILS) == 0, "TPMU_SYM_DETAILS not zero size");
#else
typedef union TPMU_SYM_DETAILS {} TPMU_SYM_DETAILS;
#endif

typedef struct TPMT_SYM_DEF {
  TPMI_ALG_SYM algorithm;
  TPMU_SYM_KEY_BITS key_bits;
  TPMU_SYM_MODE mode;
  TPMU_SYM_DETAILS details;
} TPMT_SYM_DEF;

typedef struct TPMT_SYM_DEF_OBJECT {
  TPMI_ALG_SYM_OBJECT algorithm;
  TPMU_SYM_KEY_BITS key_bits;
  TPMU_SYM_MODE mode;
  TPMU_SYM_DETAILS details;
} TPMT_SYM_DEF_OBJECT;

typedef struct TPM2B_SYM_KEY {
  UINT16 size;
  BYTE buffer[MAX_SYM_KEY_BYTES];
} TPM2B_SYM_KEY;

typedef struct TPMS_SYMCIPHER_PARMS {
  TPMT_SYM_DEF_OBJECT sym;
} TPMS_SYMCIPHER_PARMS;

typedef struct TPM2B_SENSITIVE_DATA {
  UINT16 size;
  BYTE buffer[MAX_SYM_DATA];
} TPM2B_SENSITIVE_DATA;

typedef struct TPMS_SENSITIVE_CREATE {
  TPM2B_AUTH user_auth;
  TPM2B_SENSITIVE_DATA data;
} TPMS_SENSITIVE_CREATE;

typedef struct TPM2B_SENSITIVE_CREATE {
  UINT16 size;
  TPMS_SENSITIVE_CREATE sensitive;
} TPM2B_SENSITIVE_CREATE;

typedef struct TPMS_SCHEME_SIGHASH {
  TPMI_ALG_HASH hash_alg;
} TPMS_SCHEME_SIGHASH;

typedef struct TPMS_SCHEME_XOR {
  TPMI_ALG_HASH hash_alg;
  TPMI_ALG_KDF kdf;
} TPMS_SCHEME_XOR;

typedef union TPMU_SCHEME_KEYEDHASH {
  TPMS_SCHEME_HMAC hmac;
  TPMS_SCHEME_XOR xor_;
} TPMU_SCHEME_KEYEDHASH;

typedef struct TPMT_KEYEDHASH_SCHEME {
  TPMI_ALG_KEYEDHASH_SCHEME scheme;
  TPMU_SCHEME_KEYEDHASH details;
} TPMT_KEYEDHASH_SCHEME;

typedef struct TPMS_SCHEME_ECDAA {
  TPMI_ALG_HASH hash_alg;
  UINT16 count;
} TPMS_SCHEME_ECDAA;

typedef union TPMU_SIG_SCHEME {
  TPMS_SCHEME_RSASSA rsassa;
  TPMS_SCHEME_RSAPSS rsapss;
  TPMS_SCHEME_ECDSA ecdsa;
  TPMS_SCHEME_SM2 sm2;
  TPMS_SCHEME_ECDAA ecdaa;
  TPMS_SCHEME_ECSCHNORR ec_schnorr;
  TPMS_SCHEME_HMAC hmac;
  TPMS_SCHEME_SIGHASH any;
} TPMU_SIG_SCHEME;

typedef struct TPMT_SIG_SCHEME {
  TPMI_ALG_SIG_SCHEME scheme;
  TPMU_SIG_SCHEME details;
} TPMT_SIG_SCHEME;

typedef struct TPMS_SCHEME_OAEP {
  TPMI_ALG_HASH hash_alg;
} TPMS_SCHEME_OAEP;

typedef struct TPMS_SCHEME_ECDH {
  TPMI_ALG_HASH hash_alg;
} TPMS_SCHEME_ECDH;

typedef struct TPMS_SCHEME_MGF1 {
  TPMI_ALG_HASH hash_alg;
} TPMS_SCHEME_MGF1;

typedef struct TPMS_SCHEME_KDF1_SP800_56a {
  TPMI_ALG_HASH hash_alg;
} TPMS_SCHEME_KDF1_SP800_56a;

typedef struct TPMS_SCHEME_KDF2 {
  TPMI_ALG_HASH hash_alg;
} TPMS_SCHEME_KDF2;

typedef struct TPMS_SCHEME_KDF1_SP800_108 {
  TPMI_ALG_HASH hash_alg;
} TPMS_SCHEME_KDF1_SP800_108;

typedef union TPMU_KDF_SCHEME {
  TPMS_SCHEME_MGF1 mgf1;
  TPMS_SCHEME_KDF1_SP800_56a kdf1_sp800_56a;
  TPMS_SCHEME_KDF2 kdf2;
  TPMS_SCHEME_KDF1_SP800_108 kdf1_sp800_108;
} TPMU_KDF_SCHEME;

typedef struct TPMT_KDF_SCHEME {
  TPMI_ALG_KDF scheme;
  TPMU_KDF_SCHEME details;
} TPMT_KDF_SCHEME;

typedef union TPMU_ASYM_SCHEME {
  TPMS_SCHEME_RSASSA rsassa;
  TPMS_SCHEME_RSAPSS rsapss;
  TPMS_SCHEME_OAEP oaep;
  TPMS_SCHEME_ECDSA ecdsa;
  TPMS_SCHEME_SM2 sm2;
  TPMS_SCHEME_ECDAA ecdaa;
  TPMS_SCHEME_ECSCHNORR ec_schnorr;
  TPMS_SCHEME_ECDH ecdh;
  TPMS_SCHEME_SIGHASH any_sig;
} TPMU_ASYM_SCHEME;

typedef struct TPMT_ASYM_SCHEME {
  TPMI_ALG_ASYM_SCHEME scheme;
  TPMU_ASYM_SCHEME details;
} TPMT_ASYM_SCHEME;

typedef struct TPMT_RSA_SCHEME {
  TPMI_ALG_RSA_SCHEME scheme;
  TPMU_ASYM_SCHEME details;
} TPMT_RSA_SCHEME;

typedef struct TPMT_RSA_DECRYPT {
  TPMI_ALG_RSA_DECRYPT scheme;
  TPMU_ASYM_SCHEME details;
} TPMT_RSA_DECRYPT;

typedef struct TPM2B_PUBLIC_KEY_RSA {
  UINT16 size;
  BYTE buffer[MAX_RSA_KEY_BYTES];
} TPM2B_PUBLIC_KEY_RSA;

typedef struct TPM2B_PRIVATE_KEY_RSA {
  UINT16 size;
  BYTE buffer[MAX_RSA_KEY_BYTES / 2];
} TPM2B_PRIVATE_KEY_RSA;

typedef struct TPM2B_ECC_PARAMETER {
  UINT16 size;
  BYTE buffer[MAX_ECC_KEY_BYTES];
} TPM2B_ECC_PARAMETER;

typedef struct TPMS_ECC_POINT {
  TPM2B_ECC_PARAMETER x;
  TPM2B_ECC_PARAMETER y;
} TPMS_ECC_POINT;

typedef struct TPM2B_ECC_POINT {
  UINT16 size;
  TPMS_ECC_POINT point;
} TPM2B_ECC_POINT;

typedef struct TPMT_ECC_SCHEME {
  TPMI_ALG_ECC_SCHEME scheme;
  TPMU_SIG_SCHEME details;
} TPMT_ECC_SCHEME;

typedef struct TPMS_ALGORITHM_DETAIL_ECC {
  TPM_ECC_CURVE curve_id;
  UINT16 key_size;
  TPMT_KDF_SCHEME kdf;
  TPMT_ECC_SCHEME sign;
  TPM2B_ECC_PARAMETER p;
  TPM2B_ECC_PARAMETER a;
  TPM2B_ECC_PARAMETER b;
  TPM2B_ECC_PARAMETER g_x;
  TPM2B_ECC_PARAMETER g_y;
  TPM2B_ECC_PARAMETER n;
  TPM2B_ECC_PARAMETER h;
} TPMS_ALGORITHM_DETAIL_ECC;

typedef struct TPMS_SIGNATURE_RSASSA {
  TPMI_ALG_HASH hash;
  TPM2B_PUBLIC_KEY_RSA sig;
} TPMS_SIGNATURE_RSASSA;

typedef struct TPMS_SIGNATURE_RSAPSS {
  TPMI_ALG_HASH hash;
  TPM2B_PUBLIC_KEY_RSA sig;
} TPMS_SIGNATURE_RSAPSS;

typedef struct TPMS_SIGNATURE_ECDSA {
  TPMI_ALG_HASH hash;
  TPM2B_ECC_PARAMETER signature_r;
  TPM2B_ECC_PARAMETER signature_s;
} TPMS_SIGNATURE_ECDSA;

typedef union TPMU_SIGNATURE {
  TPMS_SIGNATURE_RSASSA rsassa;
  TPMS_SIGNATURE_RSAPSS rsapss;
  TPMS_SIGNATURE_ECDSA ecdsa;
  TPMS_SIGNATURE_ECDSA sm2;
  TPMS_SIGNATURE_ECDSA ecdaa;
  TPMS_SIGNATURE_ECDSA ecschnorr;
  TPMT_HA hmac;
  TPMS_SCHEME_SIGHASH any;
} TPMU_SIGNATURE;

typedef struct TPMT_SIGNATURE {
  TPMI_ALG_SIG_SCHEME sig_alg;
  TPMU_SIGNATURE signature;
} TPMT_SIGNATURE;

typedef union TPMU_ENCRYPTED_SECRET {
  BYTE ecc[sizeof(TPMS_ECC_POINT)];
  BYTE rsa[MAX_RSA_KEY_BYTES];
  BYTE symmetric[sizeof(TPM2B_DIGEST)];
  BYTE keyed_hash[sizeof(TPM2B_DIGEST)];
} TPMU_ENCRYPTED_SECRET;

typedef struct TPM2B_ENCRYPTED_SECRET {
  UINT16 size;
  BYTE secret[sizeof(TPMU_ENCRYPTED_SECRET)];
} TPM2B_ENCRYPTED_SECRET;

typedef struct TPMS_KEYEDHASH_PARMS {
  TPMT_KEYEDHASH_SCHEME scheme;
} TPMS_KEYEDHASH_PARMS;

typedef struct TPMS_ASYM_PARMS {
  TPMT_SYM_DEF_OBJECT symmetric;
  TPMT_ASYM_SCHEME scheme;
} TPMS_ASYM_PARMS;

typedef struct TPMS_RSA_PARMS {
  TPMT_SYM_DEF_OBJECT symmetric;
  TPMT_RSA_SCHEME scheme;
  TPMI_RSA_KEY_BITS key_bits;
  UINT32 exponent;
} TPMS_RSA_PARMS;

typedef struct TPMS_ECC_PARMS {
  TPMT_SYM_DEF_OBJECT symmetric;
  TPMT_ECC_SCHEME scheme;
  TPMI_ECC_CURVE curve_id;
  TPMT_KDF_SCHEME kdf;
} TPMS_ECC_PARMS;

typedef union TPMU_PUBLIC_PARMS {
  TPMS_KEYEDHASH_PARMS keyed_hash_detail;
  TPMS_SYMCIPHER_PARMS sym_detail;
  TPMS_RSA_PARMS rsa_detail;
  TPMS_ECC_PARMS ecc_detail;
  TPMS_ASYM_PARMS asym_detail;
} TPMU_PUBLIC_PARMS;

typedef struct TPMT_PUBLIC_PARMS {
  TPMI_ALG_PUBLIC type;
  TPMU_PUBLIC_PARMS parameters;
} TPMT_PUBLIC_PARMS;

typedef union TPMU_PUBLIC_ID {
  TPM2B_DIGEST keyed_hash;
  TPM2B_DIGEST sym;
  TPM2B_PUBLIC_KEY_RSA rsa;
  TPMS_ECC_POINT ecc;
} TPMU_PUBLIC_ID;

typedef struct TPMT_PUBLIC {
  TPMI_ALG_PUBLIC type;
  TPMI_ALG_HASH name_alg;
  TPMA_OBJECT object_attributes;
  TPM2B_DIGEST auth_policy;
  TPMU_PUBLIC_PARMS parameters;
  TPMU_PUBLIC_ID unique;
} TPMT_PUBLIC;

typedef struct TPM2B_PUBLIC {
  UINT16 size;
  TPMT_PUBLIC public_area;
} TPM2B_PUBLIC;

typedef struct TPM2B_PRIVATE_VENDOR_SPECIFIC {
  UINT16 size;
  BYTE buffer[PRIVATE_VENDOR_SPECIFIC_BYTES];
} TPM2B_PRIVATE_VENDOR_SPECIFIC;

typedef union TPMU_SENSITIVE_COMPOSITE {
  TPM2B_PRIVATE_KEY_RSA rsa;
  TPM2B_ECC_PARAMETER ecc;
  TPM2B_SENSITIVE_DATA bits;
  TPM2B_SYM_KEY sym;
  TPM2B_PRIVATE_VENDOR_SPECIFIC any;
} TPMU_SENSITIVE_COMPOSITE;

typedef struct TPMT_SENSITIVE {
  TPMI_ALG_PUBLIC sensitive_type;
  TPM2B_AUTH auth_value;
  TPM2B_DIGEST seed_value;
  TPMU_SENSITIVE_COMPOSITE sensitive;
} TPMT_SENSITIVE;

typedef struct TPM2B_SENSITIVE {
  UINT16 size;
  TPMT_SENSITIVE sensitive_area;
} TPM2B_SENSITIVE;

typedef struct _PRIVATE {
  TPM2B_DIGEST integrity_outer;
  TPM2B_DIGEST integrity_inner;
  TPMT_SENSITIVE sensitive;
} _PRIVATE;

typedef struct TPM2B_PRIVATE {
  UINT16 size;
  BYTE buffer[sizeof(_PRIVATE)];
} TPM2B_PRIVATE;

typedef struct _ID_OBJECT {
  TPM2B_DIGEST integrity_hmac;
  TPM2B_DIGEST enc_identity;
} _ID_OBJECT;

typedef struct TPM2B_ID_OBJECT {
  UINT16 size;
  BYTE credential[sizeof(_ID_OBJECT)];
} TPM2B_ID_OBJECT;

typedef struct TPMS_NV_PUBLIC {
  TPMI_RH_NV_INDEX nv_index;
  TPMI_ALG_HASH name_alg;
  TPMA_NV attributes;
  TPM2B_DIGEST auth_policy;
  UINT16 data_size;
} TPMS_NV_PUBLIC;

typedef struct TPM2B_NV_PUBLIC {
  UINT16 size;
  TPMS_NV_PUBLIC nv_public;
} TPM2B_NV_PUBLIC;

typedef struct TPM2B_CONTEXT_SENSITIVE {
  UINT16 size;
  BYTE buffer[MAX_CONTEXT_SIZE];
} TPM2B_CONTEXT_SENSITIVE;

typedef struct TPMS_CONTEXT_DATA {
  TPM2B_DIGEST integrity;
  TPM2B_CONTEXT_SENSITIVE encrypted;
} TPMS_CONTEXT_DATA;

typedef struct TPM2B_CONTEXT_DATA {
  UINT16 size;
  BYTE buffer[sizeof(TPMS_CONTEXT_DATA)];
} TPM2B_CONTEXT_DATA;

typedef struct TPMS_CONTEXT {
  UINT64 sequence;
  TPMI_DH_CONTEXT saved_handle;
  TPMI_RH_HIERARCHY hierarchy;
  TPM2B_CONTEXT_DATA context_blob;
} TPMS_CONTEXT;

typedef struct TPMS_CREATION_DATA {
  TPML_PCR_SELECTION pcr_select;
  TPM2B_DIGEST pcr_digest;
  TPMA_LOCALITY locality;
  TPM_ALG_ID parent_name_alg;
  TPM2B_NAME parent_name;
  TPM2B_NAME parent_qualified_name;
  TPM2B_DATA outside_info;
} TPMS_CREATION_DATA;

typedef struct TPM2B_CREATION_DATA {
  UINT16 size;
  TPMS_CREATION_DATA creation_data;
} TPM2B_CREATION_DATA;

typedef struct FIDO_DATA_RANGE {
  UINT16 offset;
  UINT16 size;
} FIDO_DATA_RANGE;

// TPM Object Attributes.
#define kFixedTPM ((TPMA_OBJECT)(1U << 1))
#define kFixedParent ((TPMA_OBJECT)(1U << 4))
#define kSensitiveDataOrigin ((TPMA_OBJECT)(1U << 5))
#define kUserWithAuth ((TPMA_OBJECT)(1U << 6))
#define kAdminWithPolicy ((TPMA_OBJECT)(1U << 7))
#define kNoDA ((TPMA_OBJECT)(1U << 10))
#define kRestricted ((TPMA_OBJECT)(1U << 16))
#define kDecrypt ((TPMA_OBJECT)(1U << 17))
#define kSign ((TPMA_OBJECT)(1U << 18))

// TPM NV Index Attributes, defined in TPM Spec Part 2 section 13.2.
#define TPMA_NV_PPWRITE ((TPMA_NV)(1U << 0))
#define TPMA_NV_OWNERWRITE ((TPMA_NV)(1U << 1))
#define TPMA_NV_AUTHWRITE ((TPMA_NV)(1U << 2))
#define TPMA_NV_POLICYWRITE ((TPMA_NV)(1U << 3))
#define TPMA_NV_COUNTER ((TPMA_NV)(1U << 4))
#define TPMA_NV_BITS ((TPMA_NV)(1U << 5))
#define TPMA_NV_EXTEND ((TPMA_NV)(1U << 6))
#define TPMA_NV_POLICY_DELETE ((TPMA_NV)(1U << 10))
#define TPMA_NV_WRITELOCKED ((TPMA_NV)(1U << 11))
#define TPMA_NV_WRITEALL ((TPMA_NV)(1U << 12))
#define TPMA_NV_WRITEDEFINE ((TPMA_NV)(1U << 13))
#define TPMA_NV_WRITE_STCLEAR ((TPMA_NV)(1U << 14))
#define TPMA_NV_GLOBALLOCK ((TPMA_NV)(1U << 15))
#define TPMA_NV_PPREAD ((TPMA_NV)(1U << 16))
#define TPMA_NV_OWNERREAD ((TPMA_NV)(1U << 17))
#define TPMA_NV_AUTHREAD ((TPMA_NV)(1U << 18))
#define TPMA_NV_POLICYREAD ((TPMA_NV)(1U << 19))
#define TPMA_NV_NO_DA ((TPMA_NV)(1U << 25))
#define TPMA_NV_ORDERLY ((TPMA_NV)(1U << 26))
#define TPMA_NV_CLEAR_STCLEAR ((TPMA_NV)(1U << 27))
#define TPMA_NV_READLOCKED ((TPMA_NV)(1U << 28))
#define TPMA_NV_WRITTEN ((TPMA_NV)(1U << 29))
#define TPMA_NV_PLATFORMCREATE ((TPMA_NV)(1U << 30))
#define TPMA_NV_READ_STCLEAR ((TPMA_NV)(1U << 31))

#endif  // MINI_TRUNKS_TSS_TYPES_H_
