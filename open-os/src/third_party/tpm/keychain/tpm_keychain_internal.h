// Copyright (c) 2009,2010 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef _TPM_KEYCHAIN_TPM_KEYCHAIN_INTERNAL_H_
#define _TPM_KEYCHAIN_TPM_KEYCHAIN_INTERNAL_H_

#include "tpm_keychain.h"

struct tkc_context {
    TSS_HCONTEXT hContext;
    TSS_HTPM     hTPM;
    TSS_HKEY     hSRK;
    TSS_HKEY     hKCHead;
    UINT32       hTPMUsageAuth;
    UINT32       hSRKUsageAuth;
    UINT32       hKCHeadUsageAuth;
    TSS_HPOLICY  hTPMUsagePolicy;
    TSS_HPOLICY  hSRKUsagePolicy;
    TSS_HPOLICY  hKCHeadUsagePolicy;
    UINT32       numPcrs;
};

tkc_context_t* tkc_open_context_internal(const char* tssServer,
                                         const char* ownerPassword,
                                         const char* srkPassword,
                                         const char* keychainPassword,
                                         UINT32      openFlags,
                                         UINT32      tssVersion);

void        tkc_close_context_internal(tkc_context_t** t);

TSS_RESULT  tkc_destroy_internal(tkc_context_t* t);

TSS_BOOL    tkc_verify_uuid_internal(tkc_context_t*   t,
                                     TSS_UUID*        pUuidData,
                                     TSS_KM_KEYINFO2* pKeyInfo);

TSS_RESULT  tkc_add_uuid_internal(tkc_context_t*      t,
                                 TSS_UUID             uuidData,
                                 UINT32               keyType,
                                 tkc_pcrs_selected_t* pcrsSelected,
                                 const char*          keyPassword,
                                 UINT32               tssVersion);

TSS_RESULT  tkc_remove_uuid_internal(tkc_context_t*    t,
                                          TSS_UUID     uuidData);

TSS_RESULT  tkc_list_uuid_internal(tkc_context_t*      t,
                                        TSS_UUID       uuidData,
                                        UINT32*        pKeyCount,
                                        TSS_BOOL       needVerbose);

TSS_RESULT  tkc_dump_uuid_internal(tkc_context_t*      t,
                                        TSS_HKEY       hKey,
                                        TSS_UUID       uuidData,
                                        UINT32         dumpType);

TSS_RESULT  tkc_change_password_uuid_internal(tkc_context_t* t,
                                              TSS_UUID       uuidData,
                                              const char*    oldPassword,
                                              const char*    newPassword);

TSS_RESULT  tkc_resetlock_internal(tkc_context_t* t);

#endif // _TPM_KEYCHAIN_TPM_KEYCHAIN_INTERNAL_H_
