// Copyright (c) 2009,2010 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string.h>
#include <sys/types.h>
#include <stdint.h>
#include <netinet/in.h>
#include <uuid/uuid.h>

#include <trousers/tss.h>
#include <trousers/trousers.h>

#include <memory.h>
#include <openssl/err.h>
#include <openssl/bio.h>
#include <openssl/bn.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>

#include "tpm_keychain_internal.h"
#include "util.h"

static const TSS_UUID SRK_UUID = TSS_UUID_SRK;
static const TSS_UUID KEYCHAIN_HEAD_UUID = TKC_HEAD_UUID;
static const TSS_UUID NULL_UUID = TKC_NULL_UUID;
static BYTE well_known_secret[] = TSS_WELL_KNOWN_SECRET;

tkc_context_t*
tkc_open_context_internal(const char* tssServer,
                          const char* ownerPassword,
                          const char* srkPassword,
                          const char* keychainPassword,
                          uint32_t    openFlags,
                          UINT32      tssVersion)
{
    TSS_RESULT result = TSP_ERROR(TSS_E_INTERNAL_ERROR);
    tkc_context_t* t = NULL;
    TSS_BOOL newlyCreated = FALSE;
    TSS_UNICODE* wszDestination = NULL;
    UINT32 authRetryCounter = 0;
    char passwordBuffer[TKC_AUTH_MAXLEN] = { 0 };

    t = calloc(1, sizeof(tkc_context_t));
    if (t == NULL) {
        return NULL;
    }

    result = Tspi_Context_Create(&t->hContext);
    if (result != TSS_SUCCESS) {
        TKC_syslog("Tspi_Context_Create", result);
        goto out;
    }

    if (tssServer != NULL) {
        wszDestination = TKC_utf8_to_utf16le((BYTE*)tssServer);
    }

    result = Tspi_Context_Connect(t->hContext, wszDestination);

    if (wszDestination != NULL) {
        free(wszDestination);
    }

    if (result != TSS_SUCCESS) {
        TKC_syslog("Tspi_Context_Connect", result);
        goto out;
    }

    if (openFlags & TKC_FLAG_NOKEYS) {
        goto keys_done;
    }

    result = Tspi_Context_LoadKeyByUUID(t->hContext, TSS_PS_TYPE_SYSTEM,
                                        SRK_UUID, &t->hSRK);
    if (result != TSS_SUCCESS) {
        TKC_syslog("Tspi_Context_LoadKeyByUUID", result);
        goto out;
    }

    result = TKC_auth_init_keyusage_policy(t->hContext, t->hSRK,
                                           &t->hSRKUsagePolicy,
                                           &t->hSRKUsageAuth);
    if (result != TSS_SUCCESS) {
        goto out;
    }

    result = Tspi_Context_GetKeyByUUID(t->hContext, TSS_PS_TYPE_USER,
                                       KEYCHAIN_HEAD_UUID, &t->hKCHead);

    if (openFlags & TKC_FLAG_CREATE) {

        TSS_FLAG initFlags = TSS_KEY_TYPE_STORAGE      |
                             TSS_KEY_SIZE_2048         |
                             TSS_KEY_VOLATILE          |
                             TSS_KEY_NOT_MIGRATABLE;
        if (tssVersion == TSS_TSPATTRIB_CONTEXT_VERSION_V1_2) {
            initFlags |= TSS_KEY_STRUCT_KEY12;
        }
        if (keychainPassword == NULL) {
            initFlags |= TSS_KEY_NO_AUTHORIZATION;
            t->hKCHeadUsageAuth = FALSE;
        } else {
            initFlags |= TSS_KEY_AUTHORIZATION;
            t->hKCHeadUsageAuth = TRUE;
        }

        if (TSS_ENOENT(result) == FALSE) {
            if (result == TSS_SUCCESS) {
                TKC_stderr(
                  "Keychain already exists. Use --destroy to get rid of it.\n");
            } else {
                TKC_stderr("Failed to determine if keychain already exists.\n");
                TKC_syslog("Tspi_Context_LoadKeyByUUID", result);
            }
            goto out;
        }

        // Really does not exist; create.

        result = Tspi_Context_CreateObject(t->hContext, TSS_OBJECT_TYPE_RSAKEY,
                                           initFlags, &t->hKCHead);
        if (result != TSS_SUCCESS) {
            TKC_syslog("Tspi_Context_CreateObject", result);
            goto out;
        }

        result = TKC_auth_init_keyusage_policy(t->hContext, t->hKCHead,
                                               &t->hKCHeadUsagePolicy,
                                               &t->hKCHeadUsageAuth);
        if (result != TSS_SUCCESS) {
            goto out;
        }

        if (keychainPassword != NULL) {
            if (*keychainPassword == '\0') {
                result = Tspi_Policy_SetSecret(t->hKCHeadUsagePolicy,
                                               TSS_SECRET_MODE_SHA1,
                                               sizeof(well_known_secret),
                                               well_known_secret);
            } else {
                result = Tspi_Policy_SetSecret(t->hKCHeadUsagePolicy,
                                               TSS_SECRET_MODE_PLAIN,
                                               strlen(keychainPassword),
                                               (BYTE*)keychainPassword);
            }
            if (result != TSS_SUCCESS) {
                TKC_syslog("Tspi_Policy_SetSecret", result);
                goto out;
            }
        }

        authRetryCounter = 0;

        while (1) { // SRK will be authorized in this loop
            if (TKC_auth_should_retry(t->hSRKUsageAuth, t->hSRKUsagePolicy,
                                      srkPassword, SRK_AUTH_RETRY_ID, result,
                                      &authRetryCounter) == TRUE) {
                result = Tspi_Key_CreateKey(t->hKCHead, t->hSRK, NULL_HOBJECT);
                if (result == TSS_SUCCESS) {
                    break;
                }
            } else {
                TKC_syslog("Tspi_Key_CreateKey", result);
                goto out;
            }
        }

        if (result != TSS_SUCCESS) {
            goto out;
        }

        result = Tspi_Context_RegisterKey(t->hContext, t->hKCHead,
                                          TSS_PS_TYPE_USER, KEYCHAIN_HEAD_UUID,
                                          TSS_PS_TYPE_SYSTEM, SRK_UUID);
        if (result != TSS_SUCCESS) {
            TKC_syslog("Tspi_Context_RegisterKey", result);
            goto out;
        }

        result = Tspi_Key_LoadKey(t->hKCHead, t->hSRK);
        if (result != TSS_SUCCESS) {
            TKC_syslog("Tspi_Key_LoadKey", result);
            goto out;
        }

        newlyCreated = TRUE;

    } else if (result != TSS_SUCCESS) { // failed to get key from UUID?
        if (TSS_ENOENT(result) == TRUE) {
            TKC_stderr(
                "Keychain does not exist. Use --create to create it.\n");
        } else {
            TKC_stderr("Failed to access keychain.\n");
            TKC_syslog("Tspi_Context_LoadKeyByUUID", result);
        }
        goto out;
    } else if ((openFlags & TKC_FLAG_DESTROY) &&
               (openFlags & TKC_FLAG_FORCE)) {
        // If we are destroying, just let them through.
        goto out;
    }

    if (newlyCreated == FALSE) { // which means we have to load hKCHead

        authRetryCounter = 0;

        while (1) {
            if (TKC_auth_should_retry(t->hSRKUsageAuth, t->hSRKUsagePolicy,
                                      srkPassword, SRK_AUTH_RETRY_ID, result,
                                      &authRetryCounter) == TRUE) {
                result = Tspi_Key_LoadKey(t->hKCHead, t->hSRK);
                if (result == TSS_SUCCESS) {
                    break;
                }
            } else {
                TKC_syslog("Tspi_Key_LoadKey", result);
                goto out;
            }
        }

        if (result != TSS_SUCCESS) {
            goto out;
        }

        result = TKC_auth_init_keyusage_policy(t->hContext, t->hKCHead,
                                               &t->hKCHeadUsagePolicy,
                                               &t->hKCHeadUsageAuth);
        if (result != TSS_SUCCESS) {
            goto out;
        }

        if (t->hKCHeadUsageAuth == TRUE) {

            BYTE* testData = (BYTE*)"?";
            UINT32 ulDataLength = 1;
            TSS_HENCDATA hEncData;

            result = Tspi_Context_CreateObject(t->hContext,
                                               TSS_OBJECT_TYPE_ENCDATA,
                                               TSS_ENCDATA_SEAL, &hEncData);
            if (result != TSS_SUCCESS) {
                TKC_syslog("Tspi_Context_CreateObject", result);
                goto out;
            }

            authRetryCounter = 0;

            while (1) {

                if (TKC_auth_should_retry(t->hKCHeadUsageAuth,
                                          t->hKCHeadUsagePolicy,
                                          keychainPassword,
                                          KEYCHAIN_AUTH_RETRY_ID,
                                          result, &authRetryCounter) == TRUE) {
                    result = Tspi_Data_Seal(hEncData, t->hKCHead,
                                            ulDataLength, testData, 0);
                    if (result == TSS_SUCCESS) {
                        Tspi_Context_CloseObject(t->hContext, hEncData);
                        break;
                    }
                } else {
                    Tspi_Context_CloseObject(t->hContext, hEncData);
                    goto out;
                }
            }

            if (result != TSS_SUCCESS) {
                goto out;
            }
        }
    }

keys_done:

    result = Tspi_Context_GetTpmObject(t->hContext, &t->hTPM);
    if (result != TSS_SUCCESS) {
        TKC_syslog("Tspi_Context_GetTpmObject", result);
        goto out;
    } else {
        TSS_FLAG capArea = TSS_TPMCAP_PROPERTY;
        UINT32 ulSubCapLength = sizeof(UINT32);
        UINT32 subCap = TSS_TPMCAP_PROP_PCR;
        UINT32 pulRespDataLength;
        BYTE* pNumPcrs;

        t->hTPMUsageAuth = TRUE;

        result = Tspi_Context_CreateObject(t->hContext,
                                           TSS_OBJECT_TYPE_POLICY,
                                           TSS_POLICY_USAGE,
                                           &t->hTPMUsagePolicy);
        if (result != TSS_SUCCESS) {
            TKC_syslog("Tspi_Context_CreateObject", result);
            goto out;
        }

        result = Tspi_Policy_AssignToObject(t->hTPMUsagePolicy, t->hTPM);
        if (result != TSS_SUCCESS) {
            TKC_syslog("Tspi_Policy_AssignToObject", result);
            goto out;
        }

        if (ownerPassword == NULL) {
            if (openFlags & TKC_FLAG_NEEDOWNER) {
                if (TKC_auth_getpass("TPM Owner Password: ",
                                     FALSE, passwordBuffer,
                                     TKC_AUTH_MAXLEN) != TRUE) {
                    result = TSP_ERROR(TSS_E_TSP_AUTHREQUIRED);
                    goto out;
                }
                ownerPassword = &passwordBuffer[0];
            }
        }

        if (ownerPassword != NULL) {
            if (*ownerPassword == '\0') {
                result = Tspi_Policy_SetSecret(t->hTPMUsagePolicy,
                                               TSS_SECRET_MODE_SHA1,
                                               sizeof(well_known_secret),
                                               well_known_secret);
            } else {
                result = Tspi_Policy_SetSecret(t->hTPMUsagePolicy,
                                               TSS_SECRET_MODE_PLAIN,
                                               strlen(ownerPassword),
                                               (BYTE*)ownerPassword);
            }
            if (result != TSS_SUCCESS) {
                TKC_syslog("Tspi_Policy_SetSecret", result);
                goto out;
            }
        }

        if (openFlags & TKC_FLAG_NOKEYS) {
            return t;
        }

        result = Tspi_TPM_GetCapability(t->hTPM, capArea, ulSubCapLength,
                                        (BYTE*)&subCap, &pulRespDataLength,
                                        &pNumPcrs);
        if (result == TSS_SUCCESS) {
            t->numPcrs = *(UINT32*)pNumPcrs;
            Tspi_Context_FreeMemory(t->hContext, pNumPcrs);
        } else {
            TKC_syslog("Tspi_TPM_GetCapability", result);
            // Don't bail out; continue with t->numPcrs set to 0.
            result = TSS_SUCCESS;
        }
    }

out:
    if (result != TSS_SUCCESS) {
        tkc_close_context_internal(&t);
    }

    return t;
}

void
tkc_close_context_internal(tkc_context_t** t)
{
    if (*t == NULL) {
        return;
    }

    if ((*t)->hContext != 0) {
        (void)Tspi_Context_CloseObject((*t)->hContext, (*t)->hKCHeadUsagePolicy);
        (void)Tspi_Context_CloseObject((*t)->hContext, (*t)->hKCHead);
        (void)Tspi_Context_CloseObject((*t)->hContext, (*t)->hSRKUsagePolicy);
        (void)Tspi_Context_CloseObject((*t)->hContext, (*t)->hSRK);
        (void)Tspi_Context_CloseObject((*t)->hContext, (*t)->hTPMUsagePolicy);
        (void)Tspi_Context_CloseObject((*t)->hContext, (*t)->hTPM);
        // XXX (void)Tspi_Context_FreeMemory((*t)->hContext, NULL);
        (void)Tspi_Context_Close((*t)->hContext);
    }

    free(*t);
    *t = NULL;

    return;
}

TSS_RESULT
tkc_destroy_internal(tkc_context_t* t)
{
    TSS_RESULT result;
    UINT32 pulKeyHierarchySize;
    TSS_KM_KEYINFO2* ppKeyHierarchy;
    TSS_HKEY hKey;
    UINT32 i;

    result = Tspi_Context_GetRegisteredKeysByUUID2(t->hContext,
                                                   TSS_PS_TYPE_USER, NULL,
                                                   &pulKeyHierarchySize,
                                                   &ppKeyHierarchy);
    if (result != TSS_SUCCESS) {
        TKC_syslog("Tspi_Context_GetRegisteredKeysByUUID2", result);
        return result;
    }

    for (i = 0; i < pulKeyHierarchySize; i++) {
        TSS_KM_KEYINFO2* ki = &ppKeyHierarchy[i];
        if ((ki->persistentStorageType == TSS_PS_TYPE_USER)       &&
            (ki->persistentStorageTypeParent == TSS_PS_TYPE_USER) &&
            !memcmp(&ki->parentKeyUUID, &KEYCHAIN_HEAD_UUID,
                    sizeof(TSS_UUID))) {
            result = Tspi_Context_UnregisterKey(t->hContext, TSS_PS_TYPE_USER,
                                                ki->keyUUID, &hKey);
            if (result == TSS_SUCCESS) {
                if (ki->fIsLoaded == TRUE) {
                    (void)Tspi_Key_UnloadKey(hKey);
                }
                result = Tspi_Context_CloseObject(t->hContext, hKey);
                if (result != TSS_SUCCESS) {
                    TKC_syslog("Tspi_Context_CloseObject", result);
                }
            } else {
                TKC_syslog("Tspi_Context_UnregisterKey", result);
            }
        }
    }

    result = Tspi_Context_UnregisterKey(t->hContext, TSS_PS_TYPE_USER,
                                        KEYCHAIN_HEAD_UUID, &hKey);
    if (result != TSS_SUCCESS) {
        TKC_syslog("Tspi_Context_UnregisterKey", result);
    }

    (void)Tspi_Key_UnloadKey(t->hKCHead);
    
    return TSS_SUCCESS;
}

TSS_BOOL
tkc_verify_uuid_internal(tkc_context_t* t, TSS_UUID* pUuidData,
                         TSS_KM_KEYINFO2* pKeyInfo)
{
    TSS_BOOL verification = FALSE;
    TSS_RESULT result;
    UINT32 pulKeyHierarchySize;
    TSS_KM_KEYINFO2* ppKeyHierarchy;
    TSS_KM_KEYINFO2* ki;

    if (pUuidData == NULL) {
        return FALSE;
    }

    if (memcmp(pUuidData, &KEYCHAIN_HEAD_UUID, sizeof(TSS_UUID)) == 0) {
        TKC_stderr(
            "This operation is not allowed on the keychain's root key.\n");
        return FALSE;
    }

    result = Tspi_Context_GetRegisteredKeysByUUID2(t->hContext,
                                                   TSS_PS_TYPE_USER,
                                                   pUuidData,
                                                   &pulKeyHierarchySize,
                                                   &ppKeyHierarchy);
    if (result != TSS_SUCCESS) {
        TKC_syslog("Tspi_Context_GetRegisteredKeysByUUID2", result);
        return result;
    }

    if (pulKeyHierarchySize < 3) {
        TKC_stderr("Found unexpected key hierarchy.\n");
        goto out;
    }

    ki = &ppKeyHierarchy[0];

    if ((ki->persistentStorageType != TSS_PS_TYPE_USER) ||
        (ki->persistentStorageTypeParent != TSS_PS_TYPE_USER)) {
        TKC_stderr("Found unexpected persistent store type.\n");
        goto out;
    }

    if (pKeyInfo != NULL) {
        memcpy(pKeyInfo, ki, sizeof(TSS_KM_KEYINFO2));
    }

    ki = &ppKeyHierarchy[pulKeyHierarchySize - 2];

    if ((ki->persistentStorageType != TSS_PS_TYPE_USER) ||
        (ki->persistentStorageTypeParent != TSS_PS_TYPE_SYSTEM)) {
        TKC_stderr("Found unexpected persistent store type.\n");
        goto out;
    }

    if (memcmp(&ki->keyUUID, &KEYCHAIN_HEAD_UUID, sizeof(TSS_UUID)) != 0) {
        TKC_stderr(
            "The key's penultimate parent is not the keychain's root key.\n");
        goto out;
    }

    if (memcmp(&ki->parentKeyUUID, &SRK_UUID, sizeof(TSS_UUID)) != 0) {
        TKC_stderr("The key's ultimate parent is not the Storage Root Key.\n");
        goto out;
    }

    verification = TRUE;

out:

    (void)Tspi_Context_FreeMemory(t->hContext, (BYTE*)ppKeyHierarchy);

    return verification;
}

TSS_RESULT
tkc_add_uuid_internal(tkc_context_t*       t,
                      TSS_UUID             uuidData,
                      UINT32               keyType,
                      tkc_pcrs_selected_t* pcrsSelected,
                      const char*          keyPassword,
                      UINT32               tssVersion)
{
    TSS_RESULT result;
    TSS_HKEY hKey = NULL_HOBJECT;
    TSS_HPOLICY hKeyUsagePolicy;
    TSS_HPCRS hPcrComposite = NULL_HOBJECT;
    BYTE* rgbPcrValue;
    UINT32 ulPcrLen;
    BYTE** pcrCache = NULL;
    TSS_UUID* uuid = NULL;
    UINT32 i;
    TSS_FLAG initFlags = 0;
    UINT32 dumpType = TKC_DUMP_TYPE_BLOB;

    if (pcrsSelected) {

        if (pcrsSelected->highest >= t->numPcrs) {
            TKC_stderr("This TPM does not have a PCR with index %u.\n",
                       pcrsSelected->highest);
            TKC_stderr("The highest PCR index for this TPM is %u.\n",
                       t->numPcrs - 1);
            return TSP_ERROR(TSS_E_BAD_PARAMETER);
        }

        if (tssVersion == TSS_TSPATTRIB_CONTEXT_VERSION_V1_2) {
            initFlags = TSS_PCRS_STRUCT_INFO_LONG;
        }

        result = Tspi_Context_CreateObject(t->hContext,
                                           TSS_OBJECT_TYPE_PCRS, initFlags,
                                           &hPcrComposite);
        if (result != TSS_SUCCESS) {
            TKC_syslog("Tspi_Context_CreateObject", result);
            return result;
        }

        pcrCache = (BYTE**)calloc(pcrsSelected->highest, sizeof(BYTE*));

        for (i = 0; i <= pcrsSelected->highest; i++) {
            if (!bit_test(pcrsSelected->bitmap, i)) {
                continue;
            }
            result = Tspi_TPM_PcrRead(t->hTPM, i, &ulPcrLen, &rgbPcrValue);
            if (result != TSS_SUCCESS) {
                TKC_syslog("Tspi_TPM_PcrRead", result);
                goto out;
            }
            pcrCache[i] = rgbPcrValue;
            result = Tspi_PcrComposite_SetPcrValue(hPcrComposite, i,
                                                   ulPcrLen, rgbPcrValue);
            if (result != TSS_SUCCESS) {
                TKC_syslog("Tspi_PcrComposite_SetPcrValue", result);
                goto out;
            }
            if (ulPcrLen != SHA_DIGEST_LENGTH) {
                TKC_stderr("Unexpected length %u of PCR value (expected %u).\n",
                           ulPcrLen, SHA_DIGEST_LENGTH);
                goto out;
            }
        }

        if (tssVersion == TSS_TSPATTRIB_CONTEXT_VERSION_V1_2) {
            result = Tspi_PcrComposite_SetPcrLocality(hPcrComposite,
                                                      TPM_LOC_ZERO  |
                                                      TPM_LOC_ONE   |
                                                      TPM_LOC_TWO   |
                                                      TPM_LOC_THREE |
                                                      TPM_LOC_FOUR);
            if (result != TSS_SUCCESS) {
                TKC_syslog("Tspi_PcrComposite_SetPcrLocality", result);
                goto out;
            }
        }
    }

    initFlags = TSS_KEY_SIZE_2048       |
                TSS_KEY_VOLATILE        |
                TSS_KEY_NOT_MIGRATABLE;

    switch (keyType) {
        case TKC_KEY_TYPE_SSH:
            initFlags |= TSS_KEY_TYPE_SIGNING;
            dumpType = TKC_DUMP_TYPE_SSH;
            break;

        case TKC_KEY_TYPE_SIGNING:
            initFlags |= TSS_KEY_TYPE_SIGNING;
            break;

        case TKC_KEY_TYPE_STORAGE:
            initFlags |= TSS_KEY_TYPE_STORAGE;
            break;

        case TKC_KEY_TYPE_BIND:
            initFlags |= TSS_KEY_TYPE_BIND;
            break;

        default:
            initFlags |= TSS_KEY_TYPE_LEGACY;
            break;
    }

    if (tssVersion == TSS_TSPATTRIB_CONTEXT_VERSION_V1_2) {
        initFlags |= TSS_KEY_STRUCT_KEY12;
    }
    if (keyPassword == NULL) {
        initFlags |= TSS_KEY_NO_AUTHORIZATION;
    } else {
        initFlags |= TSS_KEY_AUTHORIZATION;
    }

    result = Tspi_Context_CreateObject(t->hContext, TSS_OBJECT_TYPE_RSAKEY,
                                       initFlags, &hKey);
    if (result != TSS_SUCCESS) {
        TKC_syslog("Tspi_Context_CreateObject", result);
        goto out;
    }

    result = Tspi_Context_CreateObject(t->hContext, TSS_OBJECT_TYPE_POLICY,
                                       TSS_POLICY_USAGE,
                                       &hKeyUsagePolicy);
    if (result != TSS_SUCCESS) {
        TKC_syslog("Tspi_Context_CreateObject", result);
        goto out;
    }

    if (keyPassword != NULL) {
        if (*keyPassword == '\0') {
            result = Tspi_Policy_SetSecret(hKeyUsagePolicy,
                                           TSS_SECRET_MODE_SHA1,
                                           sizeof(well_known_secret),
                                           well_known_secret);
        } else {
            result = Tspi_Policy_SetSecret(hKeyUsagePolicy,
                                           TSS_SECRET_MODE_PLAIN,
                                           strlen(keyPassword),
                                           (BYTE*)keyPassword);
        }
        if (result != TSS_SUCCESS) {
            TKC_syslog("Tspi_Policy_SetSecret", result);
            goto out;
        }
    }

    result = Tspi_Policy_AssignToObject(hKeyUsagePolicy, hKey);
    if (result != TSS_SUCCESS) {
        TKC_syslog("Tspi_Policy_AssignToObject", result);
        goto out;
    }

    result = Tspi_Key_CreateKey(hKey, t->hKCHead, hPcrComposite);
    if (result != TSS_SUCCESS) {
        TKC_syslog("Tspi_Key_CreateKey", result);
        goto out;
    }

    if (!memcmp(&NULL_UUID, &uuidData, sizeof(TSS_UUID))) {
        result = Tspi_TPM_GetRandom(t->hTPM, (UINT32)sizeof(TSS_UUID),
                                        (BYTE**)&uuid);
            if (result != TSS_SUCCESS) {
                TKC_syslog("Tspi_TPM_GetRandom", result);
                goto out;
            }
    } else {
        uuid = calloc(1, sizeof(TSS_UUID));
        if (uuid == NULL) {
            result = TSP_ERROR(TSS_E_OUTOFMEMORY);
            TKC_stderr("Failed to allocate memory.\n");
            goto out;
        }
        memcpy(uuid, &uuidData, sizeof(TSS_UUID));
    }

    result = Tspi_Context_RegisterKey(t->hContext, hKey, TSS_PS_TYPE_USER,
                                      *uuid, TSS_PS_TYPE_USER,
                                      KEYCHAIN_HEAD_UUID);
    if (result != TSS_SUCCESS) {
        TKC_syslog("Tspi_Context_RegisterKey", result);
        (void)Tspi_Context_CloseObject(t->hContext, hKey);
        goto out;
    }

    result = tkc_dump_uuid_internal(t, hKey, *uuid, dumpType);
    if (result != TSS_SUCCESS) {
        TKC_stderr("Failed to display new key.\n");
    }

    (void)Tspi_Context_CloseObject(t->hContext, hKey);

out:
    if (uuid) {
        free(uuid);
    }

    if (pcrsSelected && pcrCache) {
        for (i = 0; i <= pcrsSelected->highest; i++) {
            if (pcrCache[i] != NULL) {
                if (result == TSS_SUCCESS) {
                    UINT32 j;
                    TKC_stdout("# pcr%-2u = ", i);
                    for (j = 0; j < SHA_DIGEST_LENGTH; j++) {
                        TKC_stdout("%02x", pcrCache[i][j]);
                    }
                    TKC_stdout("\n");
                }
                (void)Tspi_Context_FreeMemory(t->hContext, pcrCache[i]);
            }
        }
        free(pcrCache);
    }

    return result;
}

TSS_RESULT
tkc_remove_uuid_internal(tkc_context_t* t, TSS_UUID uuidData)
{
    TSS_RESULT result;
    TSS_HKEY hKey;

    result = Tspi_Context_UnregisterKey(t->hContext, TSS_PS_TYPE_USER,
                                        uuidData, &hKey);
    if (result != TSS_SUCCESS) {
        TKC_syslog("Tspi_Context_UnregisterKey", result);
        return result;
    }

    result = Tspi_Context_CloseObject(t->hContext, hKey);
    if (result != TSS_SUCCESS) {
        TKC_syslog("Tspi_Context_CloseObject", result);
        return result;
    }

    return result;
}

#define TKC_KEY_LABEL_FMT "  %-22s = "

static void
tkc_list_uuid_internal_printkey(tkc_context_t*   t,
                                TSS_KM_KEYINFO2* ki,
                                TSS_BOOL         showDetails)
{
    TSS_RESULT result;
    char uuid_string[128];
    TSS_HKEY hKey;
    UINT32 attribUint32;
    UINT32 blobLength;
    BYTE* blobData;

    result = Tspi_Context_GetKeyByUUID(t->hContext, TSS_PS_TYPE_USER,
                                       ki->keyUUID, &hKey);
    if (result == TSS_SUCCESS) {
        result = Tspi_GetAttribUint32(hKey, TSS_TSPATTRIB_KEY_INFO,
                                      TSS_TSPATTRIB_KEYINFO_USAGE,
                                      &attribUint32);
    }

    uuid_unparse_lower(*(uuid_t*)&ki->keyUUID, uuid_string);

    if (showDetails == FALSE) {
        TKC_stdout("%s %c%c %s\n", uuid_string,
                   (ki->fIsLoaded == TRUE) ? '*' : '_',
                   (ki->bAuthDataUsage == 0x01) ? '+' : '_',
                   (result != TSS_SUCCESS) ? "" :
                       unparse_key_usage(attribUint32));

        (void)Tspi_Context_CloseObject(t->hContext, hKey);
        return;
    }

    TKC_stdout("%s\n", uuid_string);
    TKC_stdout(TKC_KEY_LABEL_FMT, "Key Usage");
    TKC_stdout("%s\n",
               (result != TSS_SUCCESS) ? "" : unparse_key_usage(attribUint32));
    TKC_stdout(TKC_KEY_LABEL_FMT, "Authorization Required");
    TKC_stdout("%s\n", (ki->bAuthDataUsage == 0x01) ? "yes" : "no");

    result = Tspi_GetAttribUint32(hKey, TSS_TSPATTRIB_KEY_INFO,
                                  TSS_TSPATTRIB_KEYINFO_SIZE, &attribUint32);
    if (result == TSS_SUCCESS) {
        TKC_stdout(TKC_KEY_LABEL_FMT, "Size");
        switch (attribUint32) {
            case TSS_KEY_SIZEVAL_512BIT:
                TKC_stdout("512-bit");
                break;
            case TSS_KEY_SIZEVAL_1024BIT:
                TKC_stdout("1024-bit");
                break;
            case TSS_KEY_SIZEVAL_2048BIT:
                TKC_stdout("2048-bit");
                break;
            case TSS_KEY_SIZEVAL_4096BIT:
                TKC_stdout("4096-bit");
                break;
            case TSS_KEY_SIZEVAL_8192BIT:
                TKC_stdout("8192-bit");
                break;
            case TSS_KEY_SIZEVAL_16384BIT:
                TKC_stdout("16384-bit");
                break;
            default:
                TKC_stdout("unknown\n");
                break;
        }
        TKC_stdout("\n");
    }

    result = Tspi_GetAttribData(hKey, TSS_TSPATTRIB_KEY_INFO,
                                TSS_TSPATTRIB_KEYINFO_VERSION,
                                &blobLength, &blobData);
    if (result == TSS_SUCCESS) {
        TSS_VERSION* tvp = (TSS_VERSION*)blobData;
        TKC_stdout(TKC_KEY_LABEL_FMT, "Version");
        TKC_stdout("%hhx.%hhx.%hhx.%hhx\n", tvp->bMajor, tvp->bMinor,
                   tvp->bRevMajor, tvp->bRevMinor);
        Tspi_Context_FreeMemory(t->hContext, blobData);
    }

    result = Tspi_GetAttribUint32(hKey, TSS_TSPATTRIB_KEY_INFO,
                                  TSS_TSPATTRIB_KEYINFO_KEYSTRUCT,
                                  &attribUint32);
    if (result == TSS_SUCCESS) {
        switch (attribUint32) {
            case TSS_KEY_STRUCT_DEFAULT:
                TKC_stdout("default");
                break;
            case TSS_KEY_STRUCT_KEY:
                TKC_stdout("1.1b");
                break;
            case TSS_KEY_STRUCT_KEY12:
                TKC_stdout("1.2");
                break;
            default:
                TKC_stdout("unknown\n");
                break;
        }
        TKC_stdout("\n");
    }

    result = Tspi_GetAttribUint32(hKey, TSS_TSPATTRIB_KEY_INFO,
                                  TSS_TSPATTRIB_KEYINFO_KEYFLAGS,
                                  &attribUint32);
    if ((result == TSS_SUCCESS) && (attribUint32 != 0)) {
        int ctr = 0;
        TKC_stdout(TKC_KEY_LABEL_FMT, "Flags");
        if (attribUint32 & TSS_KEYFLAG_REDIRECTION) { // first
            TKC_stdout("redirection");
            ctr++;
        }
        if (attribUint32 & TSS_KEYFLAG_MIGRATABLE) {
            TKC_stdout("migratable%s", (ctr) ? "," : "");
            ctr++;
        }
        if (attribUint32 & TSS_KEYFLAG_VOLATILEKEY) {
            TKC_stdout("volatile%s", (ctr) ? "," : "");
            ctr++;
        }
        if (attribUint32 & TSS_KEYFLAG_CERTIFIED_MIGRATABLE) { // last
            TKC_stdout("certified-migratable");
            ctr++;
        }
        if (ctr) {
            TKC_stdout("\n");
        }
    }

    result = Tspi_GetAttribUint32(hKey, TSS_TSPATTRIB_KEY_INFO,
                                  TSS_TSPATTRIB_KEYINFO_ALGORITHM,
                                  &attribUint32);
    if (result == TSS_SUCCESS) {
        TKC_stdout(TKC_KEY_LABEL_FMT, "Algorithm");
        switch (attribUint32) {
            case TSS_ALG_RSA:
                TKC_stdout("RSA");
                break;
            case TSS_ALG_DES:
                TKC_stdout("DES");
                break;
            case TSS_ALG_3DES:
                TKC_stdout("3DES");
                break;
            case TSS_ALG_SHA:
                TKC_stdout("SHA1");
                break;
            case TSS_ALG_HMAC:
                TKC_stdout("RFC 2104 HMAC");
                break;
            case TSS_ALG_AES128:
                TKC_stdout("AES-128");
                break;
            case TSS_ALG_AES192:
                TKC_stdout("AES-192");
                break;
            case TSS_ALG_AES256:
                TKC_stdout("AES-256");
                break;
            case TSS_ALG_XOR:
                TKC_stdout("XOR with rolling nonces");
                break;
            case TSS_ALG_MGF1:
                TKC_stdout("XOR with MGF1");
                break;
            default:
                TKC_stdout("unknown");
                break;
        }
        TKC_stdout("\n");
    }

    result = Tspi_GetAttribUint32(hKey, TSS_TSPATTRIB_KEY_INFO,
                                  TSS_TSPATTRIB_KEYINFO_SIGSCHEME,
                                  &attribUint32);
    if (result == TSS_SUCCESS) {
        TKC_stdout(TKC_KEY_LABEL_FMT, "Signature Scheme");
        switch (attribUint32) {
            case TSS_SS_NONE:
                TKC_stdout("none");
                break;
            case TSS_SS_RSASSAPKCS1V15_SHA1:
                TKC_stdout("RSASSA-PKCS1-v1.5 + SHA1");
                break;
            case TSS_SS_RSASSAPKCS1V15_DER:
                TKC_stdout("RSASSA-PKCS1-v1.5 + DER");
                break;
            default:
                TKC_stdout("unknown");
                break;
        }
        TKC_stdout("\n");
    }

    result = Tspi_GetAttribUint32(hKey, TSS_TSPATTRIB_KEY_INFO,
                                  TSS_TSPATTRIB_KEYINFO_ENCSCHEME,
                                  &attribUint32);
    if (result == TSS_SUCCESS) {
        TKC_stdout(TKC_KEY_LABEL_FMT, "Encryption Scheme");
        switch (attribUint32) {
            case TSS_ES_NONE:
                TKC_stdout("none");
                break;
            case TSS_ES_RSAESPKCSV15:
                TKC_stdout("RSA_ES_PKCSV15");
                break;
            case TSS_ES_RSAESOAEP_SHA1_MGF1:
                TKC_stdout("RSA_ES_OAEP + SHA1");
                break;
            case TSS_ES_SYM_CNT:
                TKC_stdout("Symmetric Encryption CTR mode");
                break;
            case TSS_ES_SYM_OFB:
                TKC_stdout("Symmetric Encryption OFB mode");
                break;
            case TSS_ES_SYM_CBC_PKCS5PAD:
                TKC_stdout("Symmetric Encryption CBC mode");
                break;
            default:
                TKC_stdout("unknown");
                break;
        }
        TKC_stdout("\n");
    }

    result = Tspi_GetAttribUint32(hKey, TSS_TSPATTRIB_KEY_INFO,
                                  TSS_TSPATTRIB_KEYINFO_MIGRATABLE,
                                  &attribUint32);
    if (result == TSS_SUCCESS) {
        TKC_stdout(TKC_KEY_LABEL_FMT, "Migratable");
        TKC_stdout("%s\n", (attribUint32 == TRUE) ? "yes" : "no");
    }

    result = Tspi_GetAttribUint32(hKey, TSS_TSPATTRIB_KEY_INFO,
                                  TSS_TSPATTRIB_KEYINFO_CMK, &attribUint32);
    if (result == TSS_SUCCESS) {
        TKC_stdout(TKC_KEY_LABEL_FMT, "Certified Migratable");
        TKC_stdout("%s\n", (attribUint32 == TRUE) ? "yes" : "no");
    }

    TKC_stdout("\n");

    (void)Tspi_Context_CloseObject(t->hContext, hKey);

    return;
}

TSS_RESULT
tkc_list_uuid_internal(tkc_context_t* t,
                       TSS_UUID       uuidData,
                       UINT32*        pKeyCount,
                       TSS_BOOL       needVerbose)
{
    TSS_RESULT result;
    UINT32 keyCount = 0;
    UINT32 pulKeyHierarchySize;
    TSS_KM_KEYINFO2* ppKeyHierarchy;
    UINT32 i;

    //
    // Considering using Tspi_TPM_GetCapability(t->hTPM, TSS_TPMCAP_HANDLE, ...)
    // for additional information to display.
    //

    if (memcmp(&NULL_UUID, &uuidData, sizeof(TSS_UUID))) { // single key
        TSS_KM_KEYINFO2 keyInfo;
        if (tkc_verify_uuid_internal(t, &uuidData, &keyInfo) != TRUE) {
            return TSP_ERROR(TSS_E_BAD_PARAMETER);
        }
        tkc_list_uuid_internal_printkey(t, &keyInfo, TRUE);
        return TSS_SUCCESS;
    }

    *pKeyCount = 0;

    result = Tspi_Context_GetRegisteredKeysByUUID2(t->hContext,
                                                   TSS_PS_TYPE_USER, NULL,
                                                   &pulKeyHierarchySize,
                                                   &ppKeyHierarchy);
    if (result != TSS_SUCCESS) {
        TKC_syslog("Tspi_Context_GetRegisteredKeysByUUID2", result);
        return result;
    }

    for (i = 0; i < pulKeyHierarchySize; i++) {
        TSS_KM_KEYINFO2* ki = &ppKeyHierarchy[i];
        if ((ki->persistentStorageType == TSS_PS_TYPE_USER)       &&
            (ki->persistentStorageTypeParent == TSS_PS_TYPE_USER) &&
            !memcmp(&ki->parentKeyUUID, &KEYCHAIN_HEAD_UUID,
                    sizeof(TSS_UUID))) {
            
            tkc_list_uuid_internal_printkey(t, ki, needVerbose);

            keyCount++;
        }
    }

    (void)Tspi_Context_FreeMemory(t->hContext, (BYTE*)ppKeyHierarchy);

    *pKeyCount = keyCount;

    return TSS_SUCCESS;
}

TSS_RESULT
tkc_dump_uuid_internal(tkc_context_t* t,
                       TSS_HKEY       hKey,
                       TSS_UUID       uuidData,
                       UINT32         dumpType)
{
    TSS_RESULT result;
    UINT32 pulPubKeyLength = 0;
    BYTE* prgbPubKey = NULL;
    BYTE* blob_e = NULL;
    BYTE* blob_n = NULL;
    UINT32 blobLen_e;
    UINT32 blobLen_n;
    BIGNUM* e = NULL;
    BIGNUM* n = NULL;
    RSA* rsa = NULL;
    TSS_BOOL newlyCreated = TRUE;
    UINT32 foundKeyType = 0;
    char uuid_string[128] = { 0 };

    if (hKey == NULL_HOBJECT) { // Need to get it first.
        newlyCreated = FALSE;
        result = Tspi_Context_GetKeyByUUID(t->hContext, TSS_PS_TYPE_USER,
                                           uuidData, &hKey);
        if (result != TSS_SUCCESS) {
            TKC_syslog("Tspi_Context_GetKeyByUUID", result);
            return result;
        }
        uuid_unparse_lower(*(uuid_t*)&uuidData, uuid_string); 
    }

    if (!memcmp(&NULL_UUID, &uuidData, sizeof(TSS_UUID))) {
        BYTE* uuid;
        UINT32 uuidLength;
        result = Tspi_GetAttribData(hKey, TSS_TSPATTRIB_KEY_UUID, 0,
                                    &uuidLength, &uuid);
        if (result != TSS_SUCCESS) {
            TKC_syslog("Tspi_Context_GetAttribData", result);
            goto out;
        }
        uuid_unparse_lower(*(uuid_t*)&uuid, uuid_string);
        Tspi_Context_FreeMemory(t->hContext, uuid);
    } else {
        uuid_unparse_lower(*(uuid_t*)&uuidData, uuid_string); 
    }

    result = Tspi_GetAttribUint32(hKey, TSS_TSPATTRIB_KEY_INFO,
                                  TSS_TSPATTRIB_KEYINFO_USAGE, &foundKeyType);
    if (result != TSS_SUCCESS) {
        TKC_syslog("Tspi_Context_GetAttribUint32", result);
        goto out;
    }

    if (dumpType == TKC_DUMP_TYPE_SSH) {
        if ((newlyCreated == TRUE) || (foundKeyType == TSS_KEYUSAGE_SIGN) ||
            (foundKeyType == TSS_KEYUSAGE_LEGACY)) {
            goto dump_ssh;
        } else {
            if (newlyCreated == TRUE) {
                TKC_stdout("%s\n", uuid_string);
            } else {
                TKC_stderr("This key cannot be used with OpenSSH.\n");
            }
        }
    } else { // do blob dump for everything else
        if (newlyCreated == TRUE) {
            TKC_stdout("%s\n", uuid_string);
        } else {
            UINT32 blobLength;
            BYTE* blobData;
            result = Tspi_GetAttribData(hKey, TSS_TSPATTRIB_KEY_BLOB,
                                        TSS_TSPATTRIB_KEYBLOB_BLOB,
                                        &blobLength, &blobData);
            if (result != TSS_SUCCESS) {
                TKC_syslog("Tspi_Context_GetAttribData", result);
                goto out;
            }
            (void)TKC_writeout(blobData, blobLength);
            Tspi_Context_FreeMemory(t->hContext, blobData);
            goto out;
        }
    }

    goto out;

dump_ssh:

    result = Tspi_GetAttribData(hKey, TSS_TSPATTRIB_KEY_BLOB,
                                TSS_TSPATTRIB_KEYBLOB_PUBLIC_KEY,
                                &pulPubKeyLength, &prgbPubKey);
    if (result != TSS_SUCCESS) {
        TKC_syslog("Tspi_Context_GetAttribData", result);
        goto out;
    }

    result = Tspi_GetAttribData(hKey, TSS_TSPATTRIB_RSAKEY_INFO,
                                TSS_TSPATTRIB_KEYINFO_RSA_EXPONENT,
                                &blobLen_e, &blob_e);
    if (result != TSS_SUCCESS) {
        TKC_syslog("Tspi_Context_GetAttribData", result);
        goto out;
    }

    result = Tspi_GetAttribData(hKey, TSS_TSPATTRIB_RSAKEY_INFO,
                                TSS_TSPATTRIB_KEYINFO_RSA_MODULUS,
                                &blobLen_n, &blob_n);
    if (result != TSS_SUCCESS) {
        TKC_syslog("Tspi_Context_GetAttribData", result);
        goto out;
    }

    e = BN_new();
    if (e == NULL) {
        result = TSP_ERROR(TSS_E_OUTOFMEMORY);
        TKC_stderr("Failed to allocate memory.\n");
        goto out;
    }

    n = BN_new();
    if (n == NULL) {
        result = TSP_ERROR(TSS_E_OUTOFMEMORY);
        TKC_stderr("Failed to allocate memory.\n");
        goto out;
    }

    rsa = RSA_new();
    if (rsa == NULL) {
        result = TSP_ERROR(TSS_E_OUTOFMEMORY);
        TKC_stderr("Failed to allocate memory.\n");
        goto out;
    }

    BN_bin2bn(blob_e, blobLen_e, e);
    BN_bin2bn(blob_n, blobLen_n, n);
    rsa->e = e;
    rsa->n = n;

    dump_rsa_ssh(rsa, uuid_string);

out:

    if (prgbPubKey) {
        Tspi_Context_FreeMemory(t->hContext, prgbPubKey);
    }

    if (blob_e) {
        Tspi_Context_FreeMemory(t->hContext, blob_e);
    }

    if (blob_n) {
        Tspi_Context_FreeMemory(t->hContext, blob_n);
    }


    if (rsa) {
        RSA_free(rsa);
    } else {
        if (e) {
            BN_free(e);
        }
        if (n) {
            BN_free(n);
        }
    }

    return result;
}

static TSS_RESULT
tkc_change_password_head_internal(tkc_context_t* t,
                                  const char*    newPassword)
{
    TSS_RESULT result;
    TSS_HPOLICY hNewUsagePolicy;

    if ((newPassword != NULL) && (t->hKCHeadUsageAuth == FALSE)) {
        TKC_stderr("Unable to set password on a key that was "
                   "created to have no password.\n");
        return TSP_ERROR(TSS_E_POLICY_NO_SECRET);
    }

    // If we have come thus far, the keychain password must have been
    // verified if it did have a password. Moreover, hSRK and hKCHead
    // should be loaded.

    result = Tspi_Context_CreateObject(t->hContext, TSS_OBJECT_TYPE_POLICY,
                                       TSS_POLICY_USAGE, &hNewUsagePolicy);
    if (result != TSS_SUCCESS) {
        TKC_syslog("Tspi_Context_CreateObject", result);
        return result;
    }

    if (newPassword == NULL) {
        result = Tspi_Policy_SetSecret(hNewUsagePolicy, TSS_SECRET_MODE_NONE,
                                       0, NULL);
    } else if (*newPassword == '\0') {
        result = Tspi_Policy_SetSecret(hNewUsagePolicy,
                                       TSS_SECRET_MODE_SHA1,
                                       sizeof(well_known_secret),
                                       well_known_secret);
    } else {
        result = Tspi_Policy_SetSecret(hNewUsagePolicy,
                                       TSS_SECRET_MODE_PLAIN,
                                       strlen(newPassword),
                                       (BYTE*)newPassword);
    }

    result = Tspi_ChangeAuth(t->hKCHead, t->hSRK, hNewUsagePolicy);
    if (result != TSS_SUCCESS) {
        TKC_syslog("Tspi_ChangeAuth", result);
        goto out;
    }

    if (newPassword == NULL) {
        // Since this is not a new key, the auth usage property is not
        // editable. Therefore, even though we have "removed" the password,
        // upon a subsequent load/use of this key, it'll still look like the
        // key has a password. However, simply hitting ENTER at the password
        // prompt will suffice.
    }

    (void)Tspi_Policy_FlushSecret(t->hKCHeadUsagePolicy);

    (void)Tspi_Context_CloseObject(t->hContext, t->hKCHeadUsagePolicy);

    t->hKCHeadUsagePolicy = hNewUsagePolicy;

    result = tkc_remove_uuid_internal(t, KEYCHAIN_HEAD_UUID);
    if (result != TSS_SUCCESS) {
        goto out;
    }

    result = Tspi_Context_RegisterKey(t->hContext, t->hKCHead,
                                      TSS_PS_TYPE_USER, KEYCHAIN_HEAD_UUID,
                                      TSS_PS_TYPE_SYSTEM, SRK_UUID);
    if (result != TSS_SUCCESS) {
        TKC_syslog("Tspi_Context_RegisterKey", result);
        goto out;
    }

out:

    if ((hNewUsagePolicy != NULL_HOBJECT) && (result != TSS_SUCCESS)) {
        (void)Tspi_Context_CloseObject(t->hContext, hNewUsagePolicy);
    }

    return result;
}

TSS_RESULT
tkc_change_password_uuid_internal(tkc_context_t* t,
                                  TSS_UUID       uuidData,
                                  const char*    oldPassword,
                                  const char*    newPassword)
{
    TSS_RESULT result;
    UINT32 hOldUsageAuth;
    TSS_HPOLICY hOldUsagePolicy = NULL_HOBJECT;
    TSS_HPOLICY hNewUsagePolicy = NULL_HOBJECT;
    TSS_HKEY hKey;
    UINT32 authRetryCounter;

    if (!memcmp(&uuidData, &KEYCHAIN_HEAD_UUID, sizeof(TSS_UUID))) {
        return tkc_change_password_head_internal(t, newPassword);
    }

    result = Tspi_Context_CreateObject(t->hContext, TSS_OBJECT_TYPE_POLICY,
                                       TSS_POLICY_USAGE, &hNewUsagePolicy);
    if (result != TSS_SUCCESS) {
        TKC_syslog("Tspi_Context_CreateObject", result);
        return result;
    }

    if (newPassword == NULL) {
        result = Tspi_Policy_SetSecret(hNewUsagePolicy, TSS_SECRET_MODE_NONE,
                                       0, NULL);
    } else if (*newPassword == '\0') {
        result = Tspi_Policy_SetSecret(hNewUsagePolicy,
                                       TSS_SECRET_MODE_SHA1,
                                       sizeof(well_known_secret),
                                       well_known_secret);
    } else {
        result = Tspi_Policy_SetSecret(hNewUsagePolicy,
                                       TSS_SECRET_MODE_PLAIN,
                                       strlen(newPassword),
                                       (BYTE*)newPassword);
    }

    if (result != TSS_SUCCESS) {
        TKC_syslog("Tspi_Context_SetSecret", result);
        goto out;
    }

    result = Tspi_Context_LoadKeyByUUID(t->hContext, TSS_PS_TYPE_USER,
                                        uuidData, &hKey);
    if (result != TSS_SUCCESS) {
        TKC_syslog("Tspi_Context_LoadKeyByUUID", result);
        goto out;
    }

    result = TKC_auth_init_keyusage_policy(t->hContext, hKey,
                                           &hOldUsagePolicy, &hOldUsageAuth);
    if (result != TSS_SUCCESS) {
        goto out;
    }

    if (hOldUsageAuth == FALSE) {
        TKC_stderr("Unable to set password on a key that was "
                   "created to have no password.\n");
        result = TSP_ERROR(TSS_E_POLICY_NO_SECRET);
        goto out;
    }

    authRetryCounter = 0;

    while (1) {
        if (TKC_auth_should_retry(hOldUsageAuth, hOldUsagePolicy,
                                  oldPassword, KEY_AUTH_RETRY_ID, result,
                                  &authRetryCounter) == TRUE) {
            result = Tspi_ChangeAuth(hKey, t->hKCHead, hNewUsagePolicy);
            if (result == TSS_SUCCESS) {
                break;
            }
        } else {
            TKC_syslog("Tspi_ChangeAuth", result);
            goto out;
        }
    }

    if (result != TSS_SUCCESS) {
        goto out;
    }

    if (newPassword == NULL) {
        // Since this is not a new key, the auth usage property is not
        // editable. Therefore, even though we have "removed" the password,
        // upon a subsequent load/use of this key, it'll still look like the
        // key has a password. However, simply hitting ENTER at the password
        // prompt will suffice.
    }

    result = tkc_remove_uuid_internal(t, uuidData);
    if (result != TSS_SUCCESS) {
        goto out;
    }

    result = Tspi_Context_RegisterKey(t->hContext, hKey,
                                      TSS_PS_TYPE_USER, uuidData,
                                      TSS_PS_TYPE_USER, KEYCHAIN_HEAD_UUID);
    if (result != TSS_SUCCESS) {
        TKC_syslog("Tspi_Context_RegisterKey", result);
        goto out;
    }

out:
    if (hOldUsagePolicy != NULL_HOBJECT) {
        (void)Tspi_Context_CloseObject(t->hContext, hOldUsagePolicy);
    }

    if (hNewUsagePolicy != NULL_HOBJECT) {
        (void)Tspi_Context_CloseObject(t->hContext, hNewUsagePolicy);
    }

    if (hKey != NULL_HOBJECT) {
        (void)Tspi_Context_CloseObject(t->hContext, hKey);
    }

    return result;
}

TSS_RESULT
tkc_resetlock_internal(tkc_context_t* t)
{
    // We must have come in here with the owner password already auth'd

    TSS_RESULT result;

    result = Tspi_TPM_SetStatus(t->hTPM, TSS_TPMSTATUS_RESETLOCK, TRUE);
    if (result != TSS_SUCCESS) {
        TKC_syslog("Tspi_TPM_SetStatus", result);
        return result;
    }

    return result;
}
