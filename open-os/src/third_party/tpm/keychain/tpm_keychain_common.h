// Copyright (c) 2009,2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef _TPM_KEYCHAIN_TPM_KEYCHAIN_COMMON_H_
#define _TPM_KEYCHAIN_TPM_KEYCHAIN_COMMON_H_

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <uuid/uuid.h>

#include <trousers/tss.h>
#include <trousers/trousers.h>

#define TKC_stderr(fmt, ...) fprintf(stderr, fmt, ## __VA_ARGS__)
#define TKC_stdout(fmt, ...) fprintf(stdout, fmt, ## __VA_ARGS__)
#define TKC_syslog(tag, result) \
  fprintf(stderr, "%s[%s.%d]: %s: %s.\n", \
          __FUNCTION__, __FILE__, __LINE__, tag, Trspi_Error_String(result))
#define TKC_writeout(buf, len) write(STDOUT_FILENO, buf, len)

#define TKC_SERVER_ENVIRONMENT_VARIABLE "TSS_SERVER"

#define TKC_SSH_UUID_TAG "tpm-uuid-ssh"
#define TKC_NULL_UUID { 0, 0, 0, 0, 0, { 0, 0, 0, 0, 0, 0 } }
#define TKC_HEAD_UUID \
    { 'T', 'P', 'M', '_', '_', { 'K', 'E', 'Y', 'C', 'H', 'N' } }

#define TKC_MAX_PCRS  24ULL

#ifndef NULL_HOBJECT
#define NULL_HOBJECT 0
#endif

typedef enum {
    SRK_AUTH_RETRY_ID      = 0, 
    KEYCHAIN_AUTH_RETRY_ID = 1,
    KEY_AUTH_RETRY_ID      = 2,
    MAX_AUTH_RETRY_ID      = 2,
} tkc_auth_retry_id_t;

#define TKC_AUTH_RETRIES  3
#define TKC_AUTH_MAXLEN   2048

TSS_BOOL TSS_EACCES(TSS_RESULT failureStatus);
TSS_BOOL TSS_EEXIST(TSS_RESULT failureStatus);
TSS_BOOL TSS_ENOENT(TSS_RESULT failureStatus);

TSS_BOOL TKC_auth_getpass(const char* prompt, TSS_BOOL verify,
                          char* buf, int length);
TSS_BOOL TKC_auth_should_retry(UINT32              hUsageAuth,
                               TSS_HPOLICY         hUsagePolicy,
                               const char*         initialPassword,
                               tkc_auth_retry_id_t rid,
                               TSS_RESULT          failureStatus,
                               UINT32*             retryCounter);
TSS_RESULT TKC_auth_init_keyusage_policy(TSS_HCONTEXT hContext,
                                         TSS_HKEY     hKey,
                                         TSS_HPOLICY* phPolicy,
                                         UINT32*      phUsageAuth);
uint16_t* TKC_utf8_to_utf16le(BYTE* str);

#if defined (__APPLE__)

#include <libkern/OSByteOrder.h>

#elif defined (__linux__)

#include <endian.h>
#include <asm/byteorder.h>

#define OSSwapLittleToHostInt64(x) __le64_to_cpu(x)
#define OSSwapBigToHostInt64(x)    __be64_to_cpu(x)
#define OSSwapHostToLittleInt64(x) __cpu_to_le64(x)
#define OSSwapHostToBigInt64(x)    __cpu_to_be64(x)

#define OSSwapLittleToHostInt32(x) __le32_to_cpu(x)
#define OSSwapBigToHostInt32(x)    __be32_to_cpu(x)
#define OSSwapHostToLittleInt32(x) __cpu_to_le32(x)
#define OSSwapHostToBigInt32(x)    __cpu_to_be32(x)

#define OSSwapLittleToHostInt16(x) __le16_to_cpu(x)
#define OSSwapBigToHostInt16(x)    __be16_to_cpu(x)
#define OSSwapHostToLittleInt16(x) __cpu_to_le16(x)
#define OSSwapHostToBigInt16(x)    __cpu_to_be16(x)

#else // either *BSD or DIY

#include <sys/endian.h>

#if BYTE_ORDER == LITTLE_ENDIAN
#define __LITTLE_ENDIAN__ 1
#elif BYTE_ORDER == BIG_ENDIAN
#define __BIG_ENDIAN__ 1
#else
#error Endian Problem
#endif

#define OSSwapLittleToHostInt64(x) le64toh(x)
#define OSSwapBigToHostInt64(x)    be64toh(x)
#define OSSwapLittleToHostInt32(x) le32toh(x)
#define OSSwapBigToHostInt32(x)    be32toh(x)
#define OSSwapLittleToHostInt16(x) le16toh(x)
#define OSSwapBigToHostInt16(x)    be16toh(x)

#endif

#endif // _TPM_KEYCHAIN_TPM_KEYCHAIN_COMMON_H_
