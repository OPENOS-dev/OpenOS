// Copyright (c) 2009 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef _TPM_NV_TPM_NV_COMMON_H_
#define _TPM_NV_TPM_NV_COMMON_H_

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>

#include <trousers/tss.h>
#include <trousers/trousers.h>

#define TNV_stderr(fmt, ...) fprintf(stderr, fmt, ## __VA_ARGS__)
#define TNV_stdout(fmt, ...) fprintf(stdout, fmt, ## __VA_ARGS__)
#define TNV_syslog(tag, result) \
  fprintf(stderr, "%s[%s.%d]: %s: %s.\n", \
          __FUNCTION__, __FILE__, __LINE__, tag, Trspi_Error_String(result))
#define TNV_writeout(buf, len) write(STDOUT_FILENO, buf, len)

#define TNV_SERVER_ENVIRONMENT_VARIABLE "TSS_SERVER"

#define TNV_MAX_PCRS    24ULL
#define TNV_MAX_NV_SIZE 20480

#ifndef NULL_HOBJECT
#define NULL_HOBJECT 0
#endif

TSS_BOOL TSS_EACCES(TSS_RESULT failureStatus);
TSS_BOOL TSS_EEXIST(TSS_RESULT failureStatus);
TSS_BOOL TSS_ENOENT(TSS_RESULT failureStatus);
TSS_BOOL TSS_EAUTH(TSS_RESULT failureStatus);

uint16_t* TNV_utf8_to_utf16le(BYTE* str);

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

#endif // _TPM_NV_TPM_NV_COMMON_H_
