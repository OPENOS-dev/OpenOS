// Copyright (c) 2009,2010 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <stdint.h>
#include <netinet/in.h>
#include <uuid/uuid.h>

#include <trousers/tss.h>
#include <trousers/trousers.h>

#include "tpm_keychain.h"
#include "tpm_keychain_internal.h"

tkc_context_t*
tkc_open_context(const char* tss_server,
                 const char* owner_password,
                 const char* srk_password,
                 const char* keychain_password,
                 uint32_t    open_flags,
                 uint32_t    tss_version)
{
    return tkc_open_context_internal(tss_server, owner_password, srk_password,
                                     keychain_password, (UINT32)open_flags,
                                     (UINT32)tss_version);
}

void
tkc_close_context(tkc_context_t** t)
{
    return tkc_close_context_internal(t);
}

int32_t
tkc_destroy(tkc_context_t* t)
{
    TSS_RESULT result = tkc_destroy_internal(t);

    return (result == TSS_SUCCESS) ? 0 : -1;
}

int32_t
tkc_verify_uuid(tkc_context_t* t, const char* uuid_string)
{
    TSS_UUID uuidData;
    TSS_BOOL result;

    if (uuid_parse(uuid_string, *(uuid_t*)&uuidData) != 0) {
        TKC_stderr("Invalid UUID %s.\n", uuid_string);
        return -1;
    }

    result = tkc_verify_uuid_internal(t, &uuidData, NULL);

    return (result == TRUE) ? 0 : -1;
}

int32_t
tkc_add_uuid(tkc_context_t* t, const char* uuid_string,
             uint32_t key_type, tkc_pcrs_selected_t* pcrs_selected,
             const char* key_password, uint32_t tss_version)
{
    TSS_UUID uuidData = TKC_NULL_UUID;
    TSS_RESULT result;

    if (uuid_string != NULL) {
        if (uuid_parse(uuid_string, *(uuid_t*)&uuidData) != 0) {
            TKC_stderr("Invalid UUID %s.\n", uuid_string);
            return -1;
        }
    }

    result = tkc_add_uuid_internal(t, uuidData, (UINT32)key_type,
                                   pcrs_selected, key_password,
                                   (UINT32)tss_version);

    return (result == TSS_SUCCESS) ? 0 : -1;
}

int32_t
tkc_remove_uuid(tkc_context_t* t, const char* uuid_string)
{
    TSS_UUID uuidData;
    TSS_RESULT result;

    if (uuid_parse(uuid_string, *(uuid_t*)&uuidData) != 0) {
        TKC_stderr("Invalid UUID %s.\n", uuid_string);
        return -1;
    }

    if (tkc_verify_uuid_internal(t, &uuidData, NULL) != TRUE) {
        return -1;
    }

    result = tkc_remove_uuid_internal(t, uuidData);

    return (result == TSS_SUCCESS) ? 0 : -1;
}

int32_t
tkc_list_uuid(tkc_context_t* t, const char* uuid_string, uint32_t verbose)
{
    TSS_UUID uuidData = TKC_NULL_UUID;
    TSS_RESULT result;
    TSS_BOOL singleKey = FALSE;
    UINT32 keyCount;

    if (uuid_string != NULL) {
        if (strcmp(uuid_string, TKC_SRK_NAME) == 0) {
            TSS_UUID SRK_UUID = TSS_UUID_SRK;
            memcpy(&uuidData, &SRK_UUID, sizeof(TSS_UUID));
        } else if (strcmp(uuid_string, TKC_HEAD_NAME) == 0) {
            TSS_UUID KEYCHAIN_HEAD_UUID = TKC_HEAD_UUID;
            memcpy(&uuidData, &KEYCHAIN_HEAD_UUID, sizeof(TSS_UUID));
        } else {
            if (uuid_parse(uuid_string, *(uuid_t*)&uuidData) != 0) {
                TKC_stderr("Invalid UUID %s.\n", uuid_string);
                return -1;
            }
            if (tkc_verify_uuid_internal(t, &uuidData, NULL) != TRUE) {
                return -1;
            }
        }
        singleKey = TRUE;
    }

    result = tkc_list_uuid_internal(t, uuidData, &keyCount,
                                    (verbose == 0) ? FALSE : TRUE);

    if (result == TSS_SUCCESS) {
        if ((singleKey == FALSE) && (keyCount > 0)) {
            TKC_stdout("%u key%s", keyCount, (keyCount > 1) ? "s\n" : "\n");
        }
        return 0;
    }

    return -1;
}

int32_t
tkc_dump_uuid(tkc_context_t* t, const char* uuid_string)
{
    TSS_UUID uuidData;
    TSS_RESULT result;

    if (uuid_parse(uuid_string, *(uuid_t*)&uuidData) != 0) {
        TKC_stderr("Invalid UUID %s.\n", uuid_string);
        return -1;
    }

    if (tkc_verify_uuid_internal(t, &uuidData, NULL) != TRUE) {
        return -1;
    }

    result = tkc_dump_uuid_internal(t, NULL_HOBJECT, uuidData,
                                    TKC_DUMP_TYPE_BLOB);

    return (result == TSS_SUCCESS) ? 0 : -1;
}

int32_t
tkc_ssh_uuid(tkc_context_t* t, const char* uuid_string)
{
    TSS_UUID uuidData;
    TSS_RESULT result;

    if (uuid_parse(uuid_string, *(uuid_t*)&uuidData) != 0) {
        TKC_stderr("Invalid UUID %s.\n", uuid_string);
        return -1;
    }

    if (tkc_verify_uuid_internal(t, &uuidData, NULL) != TRUE) {
        return -1;
    }

    result = tkc_dump_uuid_internal(t, NULL_HOBJECT, uuidData,
                                    TKC_DUMP_TYPE_SSH);

    return (result == TSS_SUCCESS) ? 0 : -1;
}

int32_t
tkc_change_password_uuid(tkc_context_t* t,
                         const char* uuid_string,
                         const char* old_password,
                         const char* new_password)
{
    TSS_UUID uuidData = TKC_HEAD_UUID;
    TSS_RESULT result;

    if (strcmp(uuid_string, TKC_HEAD_NAME) == 0) {
        // nothing
    } else {
        if (uuid_parse(uuid_string, *(uuid_t*)&uuidData) != 0) {
            TKC_stderr("Invalid UUID %s.\n", uuid_string);
            return -1;
        }
        if (tkc_verify_uuid_internal(t, &uuidData, NULL) != TRUE) {
            return -1;
        }
    }

    result = tkc_change_password_uuid_internal(t, uuidData, old_password,
                                               new_password);

    return (result == TSS_SUCCESS) ? 0 : -1;
}

int32_t
tkc_resetlock(tkc_context_t* t)
{
    TSS_RESULT result = tkc_resetlock_internal(t);

    return (result == TSS_SUCCESS) ? 0 : -1;
}
