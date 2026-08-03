// Copyright (c) 2009,2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <tpm_keychain_common.h>
#include <openssl/evp.h>

static BYTE well_known_secret[] = TSS_WELL_KNOWN_SECRET;

typedef struct {
    const char* password_prompt;
    const char* well_known_envvar;
} tkc_auth_retry_data_t;

static tkc_auth_retry_data_t tkc_auth_retry_data[] = {
    { "Storage Root Key Password: ", "SRK_USES_WELL_KNOWN_SECRET"      },
    { "Keychain Password: ",         "KEYCHAIN_USES_WELL_KNOWN_SECRET" },
    { "Key Password: ",              "KEY_USES_WELL_KNOWN_SECRET"      },
};

TSS_BOOL
TKC_auth_getpass(const char* prompt, TSS_BOOL verify,
                 char* buf, int length)
{
    if (EVP_read_pw_string(buf, length, prompt, (verify == TRUE) ? 1 : 0)) {
        return FALSE;
    }

    return TRUE;
}

TSS_BOOL
TKC_auth_should_retry(UINT32              hUsageAuth,
                      TSS_HPOLICY         hUsagePolicy,
                      const char*         initialPassword,
                      tkc_auth_retry_id_t rid,
                      TSS_RESULT          failureStatus,
                      UINT32*             retryCounter)
{
    TSS_RESULT result;
    char passwordBuffer[TKC_AUTH_MAXLEN] = { 0 };
    size_t passwordLen;
    const char* passwordPointer = NULL;

    if (rid > MAX_AUTH_RETRY_ID) {
        return FALSE;
    }

    if (*retryCounter > TKC_AUTH_RETRIES) {
        return FALSE;
    }

    if (hUsageAuth == FALSE) {
        // In this case, we want the caller to do two things: firstly, not call
        // us again (which is why we up the value of the retry counter), and
        // secondly, to go ahead with whatever operation they intend to do
        // (which is why we return TRUE).
        *retryCounter = TKC_AUTH_RETRIES + 1;
        return TRUE;
    }

    if (*retryCounter == 0) { // haven't attempted anything yet
        *retryCounter = 1;
        if (initialPassword != NULL) { // we were given some password initially
            passwordPointer = initialPassword;
            goto read_password_done;
        } else if (getenv(tkc_auth_retry_data[rid].well_known_envvar) != NULL) {
            passwordPointer = (char*)&well_known_secret[0];
            goto read_password_done;
        } else {
            goto read_password;
        }
    }

    if (TSS_EACCES(failureStatus) != TRUE) {
        return FALSE;
    }

    *retryCounter += 1;

read_password:

    if (EVP_read_pw_string(passwordBuffer, TKC_AUTH_MAXLEN,
                           tkc_auth_retry_data[rid].password_prompt, 0)) {
        return FALSE;
    }

    passwordPointer = &passwordBuffer[0];

read_password_done:

    passwordLen = strlen(passwordPointer);
    if (passwordLen >= TKC_AUTH_MAXLEN) {
        return FALSE;
    }

    if (passwordLen == 0) {
        result = Tspi_Policy_SetSecret(hUsagePolicy, TSS_SECRET_MODE_SHA1,
                                       sizeof(well_known_secret),
                                       well_known_secret);
    } else {
        result = Tspi_Policy_SetSecret(hUsagePolicy, TSS_SECRET_MODE_PLAIN,
                                       passwordLen, (BYTE*)passwordPointer);
        memset(passwordBuffer, 0, passwordLen);
    }

    if (result != TSS_SUCCESS) {
        TKC_syslog("Tspi_Policy_SetSecret", result);
    }

    return (result == TSS_SUCCESS) ? TRUE : FALSE;
}

TSS_RESULT
TKC_auth_init_keyusage_policy(TSS_HCONTEXT hContext,
                              TSS_HKEY     hKey,
                              TSS_HPOLICY* phPolicy,
                              UINT32*      phUsageAuth)
{
    TSS_RESULT result;

    result = Tspi_GetAttribUint32(hKey, TSS_TSPATTRIB_KEY_INFO,
                                  TSS_TSPATTRIB_KEYINFO_AUTHUSAGE,
                                  phUsageAuth);
    if (result != TSS_SUCCESS) {
        TKC_syslog("Tspi_GetAttribUint32", result);
        return result;
    }

    result = Tspi_Context_CreateObject(hContext, TSS_OBJECT_TYPE_POLICY,
                                       TSS_POLICY_USAGE, phPolicy);
    if (result != TSS_SUCCESS) {
        TKC_syslog("Tspi_Context_CreateObject", result);
        return result;
    }

    result = Tspi_Policy_SetSecret(*phPolicy, TSS_SECRET_MODE_NONE, 0, NULL);
    if (result != TSS_SUCCESS) {
        TKC_syslog("Tspi_Policy_SetSecret", result);
        (void)Tspi_Context_CloseObject(hContext, *phPolicy);
        *phPolicy = NULL_HOBJECT;
        return result;
    }

    result = Tspi_Policy_AssignToObject(*phPolicy, hKey);
    if (result != TSS_SUCCESS) {
        TKC_syslog("Tspi_Policy_AssignToObject", result);
        (void)Tspi_Context_CloseObject(hContext, *phPolicy);
        *phPolicy = NULL_HOBJECT;
    }

    return result;
}

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

uint16_t*
TKC_utf8_to_utf16le(BYTE* str) // dummy implementation for now
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
