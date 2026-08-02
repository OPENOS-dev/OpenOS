// Copyright (c) 2009,2010 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef _TPM_KEYCHAIN_TPM_KEYCHAIN_H_
#define _TPM_KEYCHAIN_TPM_KEYCHAIN_H_

#include <tpm_keychain_common.h>
#include "bitstring.h"

typedef struct {
    uint32_t count;
    uint32_t highest;
    bitstr_t bit_decl(bitmap, TKC_MAX_PCRS);
} tkc_pcrs_selected_t;

struct tkc_context;

typedef struct tkc_context tkc_context_t;

enum {
    TKC_KEY_TYPE_NONE       = -1,
    TKC_KEY_TYPE_SSH        = 0,
    TKC_KEY_TYPE_SIGNING    = 1,
    TKC_KEY_TYPE_STORAGE    = 2,
    TKC_KEY_TYPE_IDENTITY   = 3,
    TKC_KEY_TYPE_AUTHCHANGE = 4,
    TKC_KEY_TYPE_BIND       = 5,
    TKC_KEY_TYPE_LEGACY     = 6,
    TKC_KEY_TYPE_MIGRATE    = 7,
};

enum {
    TKC_DUMP_TYPE_NONE      = 0,
    TKC_DUMP_TYPE_BLOB      = 1,
    TKC_DUMP_TYPE_SSH       = 2,
};

#define TKC_FLAG_FORCE     0x00000001
#define TKC_FLAG_CREATE    0x00000002
#define TKC_FLAG_DESTROY   0x00000004
#define TKC_FLAG_NOKEYS    0x00000008
#define TKC_FLAG_NEEDOWNER 0x00000010

#define TKC_SRK_NAME       "srk"
#define TKC_HEAD_NAME      "keychain"

tkc_context_t* tkc_open_context(const char* tss_server,
                                const char* owner_password,
                                const char* srk_password,
                                const char* keychain_password,
                                uint32_t    open_flags,
                                uint32_t    tss_version);
void    tkc_close_context(tkc_context_t** t);

int32_t tkc_destroy(tkc_context_t* t);
int32_t tkc_verify_uuid(tkc_context_t* t, const char* uuid_string);
int32_t tkc_add_uuid(tkc_context_t* t, const char* uuid_string,
                     uint32_t key_type, tkc_pcrs_selected_t* pcrs_selected,
                     const char* key_password, uint32_t tss_version);
int32_t tkc_remove_uuid(tkc_context_t* t, const char* uuid_string);
int32_t tkc_list_uuid(tkc_context_t* t, const char* uuid_string,
                      uint32_t verbose);
int32_t tkc_dump_uuid(tkc_context_t* t, const char* uuid_string);
int32_t tkc_ssh_uuid(tkc_context_t* t, const char* uuid_string);
int32_t tkc_change_password_uuid(tkc_context_t* t,
                                 const char* uuid_string,
                                 const char* old_password,
                                 const char* new_password);
int32_t     tkc_resetlock(tkc_context_t* t);

#endif // _TPM_KEYCHAIN_TPM_KEYCHAIN_H_
