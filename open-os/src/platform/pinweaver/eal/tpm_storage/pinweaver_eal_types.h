
/* Copyright 2021 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef __PINWEAVER_EAL_TYPES_H
#define __PINWEAVER_EAL_TYPES_H

#include <openssl/sha.h>
#include <openssl/hmac.h>
#include <stdio.h>

#define PINWEAVER_EAL_INFO(...) \
    do { \
        printf(__VA_ARGS__); \
        printf("\n"); \
    } while(0)

typedef SHA256_CTX pinweaver_eal_sha256_ctx_t;
typedef HMAC_CTX *pinweaver_eal_hmac_sha256_ctx_t;

#endif  /* __PINWEAVER_EAL_TYPES_H */
