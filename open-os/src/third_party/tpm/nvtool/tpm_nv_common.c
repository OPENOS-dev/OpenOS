// Copyright (c) 2009,2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <tpm_nv_common.h>

TSS_BOOL
TSS_EACCES(TSS_RESULT result)
{
    if (TPM_ERROR(result)) {
        switch (ERROR_CODE(result)) {
        case TPM_E_AUTHFAIL:
            return TRUE;
            break;
        case TPM_E_AUTH2FAIL:
            return TRUE;
            break;
        case TPM_E_DEFEND_LOCK_RUNNING:
        default:
            return FALSE;
            break;
        }
    }

    return FALSE;
}

TSS_BOOL
TSS_EEXIST(TSS_RESULT result)
{
    if (TSP_ERROR(result) &&
        (ERROR_CODE(result) == TSS_E_KEY_ALREADY_REGISTERED)) {
        return TRUE;
    }

    return FALSE;
}

TSS_BOOL
TSS_ENOENT(TSS_RESULT result)
{
    if (TSP_ERROR(result) &&
        (ERROR_CODE(result) == TSS_E_PS_KEY_NOTFOUND)) {
        return TRUE;
    }

    return FALSE;
}

TSS_BOOL
TSS_EAUTH(TSS_RESULT result)
{
    if (TSP_ERROR(result) &&
        (ERROR_CODE(result) == TSS_E_POLICY_NO_SECRET)) {
        return TRUE;
    }

    return FALSE;
}

uint16_t*
TNV_utf8_to_utf16le(BYTE* str) // dummy implementation for now
{
    size_t len;
    uint16_t* utf;
    const BYTE* sp;
    uint16_t* up;

    if (!str) {
        return NULL;
    }

    len = strlen((const char*)str);
    utf = (uint16_t*)calloc(1, 2 * (len + 1));

    for (sp = str, up = utf; *sp; sp++, up++) {
        *up = OSSwapHostToLittleInt16((uint16_t)*sp);
    }

    return utf;
}
