// Copyright (c) 2009,2010 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef _TPM_KEYCHAIN_UTIL_H_
#define _TPM_KEYCHAIN_UTIL_H_

#include <sys/types.h>
#include <stdint.h>
#include <openssl/rsa.h>

void        dump_rsa_ssh(RSA* rsa, char* uuid_string);
uint32_t    parse_key_type(const char* ktype);
const char* unparse_key_usage(uint32_t kusage);

#endif // _TPM_KEYCHAIN_UTIL_H_
