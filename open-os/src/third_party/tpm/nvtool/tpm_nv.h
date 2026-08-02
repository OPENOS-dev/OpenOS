// Copyright (c) 2009,2010 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef _TPM_NV_TPM_NV_H_
#define _TPM_NV_TPM_NV_H_

#include "bitstring.h"
#include "tpm_nv_common.h"

typedef struct {
    uint32_t count;
    uint32_t highest;
    bitstr_t bit_decl(bitmap, TNV_MAX_PCRS);
} tnv_pcrs_selected_t;

typedef struct {
    uint32_t            tss_version;
    uint32_t            flags;
    uint32_t            index;
    uint32_t            permissions;
    tnv_pcrs_selected_t pcrs_selected;
    const char*         index_password;
    const char*         owner_password;
    const char*         password;
    const char*         data;
    int                 data_fd;
    uint32_t            offset;
    uint32_t            size;
} tnv_args_t;

typedef struct {
    const char* permission_name;
    uint32_t    permission_value;
    TSS_BOOL    allowed;
} tpm_nv_permission_name_t;

extern tpm_nv_permission_name_t TPM_NV_PER_table[];

struct tnv_data_public_t;
struct tnv_context;

typedef struct tnv_context tnv_context_t;

#define TNV_FLAG_CREATE      0x00000002
#define TNV_FLAG_DESTROY     0x00000004
#define TNV_FLAG_NEEDOWNER   0x00000010
#define TNV_FLAG_NONSPECIFIC 0x00000020
#define TNV_FLAG_HEXDUMP     0x00000040
#define TNV_FLAG_FILEDATA    0x00000080
#define TNV_FLAG_RWAUTH      0x00000100

tnv_context_t* tnv_open_context(const char* tss_erver, tnv_args_t* a);
void           tnv_close_context(tnv_context_t** t);
int32_t        tnv_define(tnv_context_t* t, tnv_args_t* a);
int32_t        tnv_release(tnv_context_t* t, tnv_args_t* a);
int32_t        tnv_list(tnv_context_t* t, tnv_args_t* a);
int32_t        tnv_read(tnv_context_t* t, tnv_args_t* a);
int32_t        tnv_write(tnv_context_t* t, tnv_args_t* a);

#endif // _TPM_NV_TPM_NV_H_
