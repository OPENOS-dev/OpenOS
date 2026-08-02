// Copyright (c) 2009,2010 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <stdint.h>
#include <netinet/in.h>
#include <uuid/uuid.h>

#include <openssl/sha.h>

#include <trousers/tss.h>
#include <trousers/trousers.h>

#include "tpm_nv.h"

typedef struct {
    TSS_BOOL              initialized;
    TPM_NV_INDEX          nvIndex;
    TPM_NV_PER_ATTRIBUTES permission_attributes;
    TSS_BOOL              bReadSTClear;
    TSS_BOOL              bWriteSTClear;
    TSS_BOOL              bWriteDefine;
    UINT32                dataSize;
} tnv_data_public_t;

struct tnv_context {
    TSS_HCONTEXT      hContext;
    TSS_HTPM          hTPM;
    TSS_HPOLICY       hTPMUsagePolicy;
    UINT32            numPcrs;
    TSS_HNVSTORE      hNVStore;
    TSS_HPOLICY       hNVStoreUsagePolicy;
    tnv_data_public_t hNVStorePublicInfo;
};

static BYTE well_known_secret[] = TSS_WELL_KNOWN_SECRET;

tpm_nv_permission_name_t TPM_NV_PER_table[] = {
    { "READ_STCLEAR",  TPM_NV_PER_READ_STCLEAR,  TRUE  },
    { "AUTHREAD",      TPM_NV_PER_AUTHREAD,      TRUE  },
    { "OWNERREAD",     TPM_NV_PER_OWNERREAD,     TRUE  },
    { "PPREAD",        TPM_NV_PER_PPREAD,        TRUE  },
    { "GLOBALLOCK",    TPM_NV_PER_GLOBALLOCK,    FALSE },
    { "WRITE_STCLEAR", TPM_NV_PER_WRITE_STCLEAR, TRUE  },
    { "WRITEDEFINE",   TPM_NV_PER_WRITEDEFINE,   FALSE },
    { "WRITEALL",      TPM_NV_PER_WRITEALL,      TRUE  },
    { "AUTHWRITE",     TPM_NV_PER_AUTHWRITE,     TRUE  },
    { "OWNERWRITE",    TPM_NV_PER_OWNERWRITE,    TRUE  },
    { "PPWRITE",       TPM_NV_PER_PPWRITE,       TRUE  },
    { NULL, 0, FALSE },
};

#define TNV_PUB_LABEL_FMT "  %-22s = "

static TSS_BOOL
tnv_print_nv_data_public(UINT32 nvIndex, UINT32 dataLength, BYTE* data)
{
    BYTE* cursor = data;
    BYTE value8;
    UINT16 value16;
    UINT32 value32, i, j, printed, curPCR, aPCR;

    cursor += sizeof(TPM_STRUCTURE_TAG);

    value32 = OSSwapBigToHostInt32(*(UINT32*)cursor);

    if (value32 != nvIndex) {
        TNV_stderr("Failed to validate NV area public data.\n");
        return FALSE;
    }

    TNV_stdout("# NV Index %#010x\n", value32);
    cursor += sizeof(UINT32);

    // pcrInfoRead begin

    value16 = OSSwapBigToHostInt16(*(UINT16*)cursor);
    cursor += sizeof(UINT16);
    TNV_stdout(TNV_PUB_LABEL_FMT, "PCRs (read)");
    for (i = 0, printed = 0, curPCR = 0, aPCR = 0; i < value16; i++, cursor++) {
        for (j = 0; j < 8; j++) {
            if ((1 << j) & *(BYTE*)cursor) {
                if (printed) {
                    TNV_stdout(", ");
                    printed = 0;
                }
                TNV_stdout("PCR%u", curPCR);
                printed = 1;
                aPCR++;
            }
            curPCR++;
        }
    }
    if (aPCR == 0) {
        TNV_stdout("none\n");
    } else {
        TNV_stdout("\n");
    }

    TNV_stdout(TNV_PUB_LABEL_FMT, "Locality (read)");
    value8 = *(BYTE*)cursor;
    if (value8 &
        ~(TPM_LOC_ZERO | TPM_LOC_ONE | TPM_LOC_TWO | TPM_LOC_THREE |
          TPM_LOC_FOUR)) {
        TNV_stdout("unknown value (%#hhx)\n", value8);
    } else {
        for (i =0, printed = 0; i < 8; i++) {
            if ((1 << i) & value8) {
                if (printed) {
                    TNV_stdout(", ");
                    printed = 0;
                }
                TNV_stdout("%u", i);
                printed = 1;
            }
        }
    }
    TNV_stdout("\n");
    cursor += sizeof(TPM_LOCALITY_SELECTION);

    TNV_stdout(TNV_PUB_LABEL_FMT, "PCR Composite (read)");
    for (i = 0; i < TPM_SHA1_160_HASH_LEN; i++, cursor++) {
        TNV_stdout("%02hhx", *(BYTE*)cursor);
    }
    TNV_stdout("\n");

    // pcrInfoRead end

    // pcrInfoWrite begin

    value16 = OSSwapBigToHostInt16(*(UINT16*)cursor);
    cursor += sizeof(UINT16);
    TNV_stdout(TNV_PUB_LABEL_FMT, "PCRs (write)");
    for (i = 0, printed = 0, curPCR = 0, aPCR = 0; i < value16; i++, cursor++) {
        for (j = 0; j < 8; j++) {
            if ((1 << j) & *(BYTE*)cursor) {
                if (printed) {
                    TNV_stdout(", ");
                    printed = 0;
                }
                TNV_stdout("PCR%u", curPCR);
                printed = 1;
                aPCR++;
            }
            curPCR++;
        }
    }
    if (aPCR == 0) {
        TNV_stdout("none\n");
    } else {
        TNV_stdout("\n");
    }

    TNV_stdout(TNV_PUB_LABEL_FMT, "Locality (write)");
    value8 = *(BYTE*)cursor;
    if (value8 &
        ~(TPM_LOC_ZERO | TPM_LOC_ONE | TPM_LOC_TWO | TPM_LOC_THREE |
          TPM_LOC_FOUR)) {
        TNV_stdout("unknown value (%#hhx)\n", value8);
    } else {
        for (i =0, printed = 0; i < 8; i++) {
            if ((1 << i) & value8) {
                if (printed) {
                    TNV_stdout(", ");
                    printed = 0;
                }
                TNV_stdout("%u", i);
                printed = 1;
            }
        }
    }
    TNV_stdout("\n");
    cursor += sizeof(TPM_LOCALITY_SELECTION);

    TNV_stdout(TNV_PUB_LABEL_FMT, "PCR Composite (write)");
    for (i = 0; i < TPM_SHA1_160_HASH_LEN; i++, cursor++) {
        TNV_stdout("%02hhx", *(BYTE*)cursor);
    }
    TNV_stdout("\n");

    // pcrInfoWrite end

    // seek into TPM_NV_ATTRIBUTES to get to the permission value
    cursor += sizeof(TPM_STRUCTURE_TAG);

    value32 = OSSwapBigToHostInt32(*(UINT32*)cursor);
    cursor += sizeof(UINT32);
    TNV_stdout(TNV_PUB_LABEL_FMT, "Permissions");
    for (i = 0, printed = 0; TPM_NV_PER_table[i].permission_name; i++) {
        if (TPM_NV_PER_table[i].permission_value & value32) {
            if (printed) {
                TNV_stdout(", ");
                printed = 0;
            }
            TNV_stdout("%s", TPM_NV_PER_table[i].permission_name);
            printed = 1;
        }
    }
    TNV_stdout("\n");

    value8 = *(BYTE*)cursor;
    cursor += 1;
    TNV_stdout(TNV_PUB_LABEL_FMT, "bReadSTClear");
    TNV_stdout("%s\n", (value8 == TRUE) ? "yes" : "no");

    value8 = *(BYTE*)cursor;
    cursor += 1;
    TNV_stdout(TNV_PUB_LABEL_FMT, "bWriteSTClear");
    TNV_stdout("%s\n", (value8 == TRUE) ? "yes" : "no");

    value8 = *(BYTE*)cursor;
    cursor += 1;
    TNV_stdout(TNV_PUB_LABEL_FMT, "bWriteDefine");
    TNV_stdout("%s\n", (value8 == TRUE) ? "yes" : "no");

    value32 = OSSwapBigToHostInt32(*(UINT32*)cursor);
    cursor += sizeof(UINT32);

    TNV_stdout(TNV_PUB_LABEL_FMT, "Data Size");
    TNV_stdout("%u\n", value32);
    TNV_stdout("\n");

    return TRUE;
}

static TSS_BOOL
tnv_populate_nv_data_public(UINT32 nvIndex, tnv_data_public_t* publicInfo,
                            UINT32 dataLength, BYTE* data)
{
    BYTE* cursor = data;

    cursor += sizeof(TPM_STRUCTURE_TAG);

    if (OSSwapBigToHostInt32(*(UINT32*)cursor) != nvIndex) {
        return FALSE;
    }

    publicInfo->nvIndex = OSSwapBigToHostInt32(*(UINT32*)cursor);
    cursor += sizeof(UINT32);

    // skip over pcrInfoRead
    cursor += OSSwapBigToHostInt16(*(UINT16*)cursor) + sizeof(UINT16);
    cursor += sizeof(TPM_LOCALITY_SELECTION);
    cursor += sizeof(TPM_COMPOSITE_HASH);

    // skip over pcrInfoWrite
    cursor += OSSwapBigToHostInt16(*(UINT16*)cursor) + sizeof(UINT16);
    cursor += sizeof(TPM_LOCALITY_SELECTION);
    cursor += sizeof(TPM_COMPOSITE_HASH);

    // seek into TPM_NV_ATTRIBUTES to get to the permission value
    cursor += sizeof(TPM_STRUCTURE_TAG);

    publicInfo->permission_attributes = OSSwapBigToHostInt32(*(UINT32*)cursor);
    cursor += sizeof(UINT32);

    publicInfo->bReadSTClear = *(BYTE*)cursor;
    cursor += 1;

    publicInfo->bWriteSTClear = *(BYTE*)cursor;
    cursor += 1;

    publicInfo->bWriteDefine = *(BYTE*)cursor;
    cursor += 1;

    publicInfo->dataSize = OSSwapBigToHostInt32(*(UINT32*)cursor);

    return TRUE;
}

tnv_context_t*
tnv_open_context(const char* tssServer, tnv_args_t* a)
{
    TSS_RESULT result = TSP_ERROR(TSS_E_INTERNAL_ERROR);
    tnv_context_t* t = NULL;
    TSS_UNICODE* wszDestination = NULL;
    UINT32 subCap = TSS_TPMCAP_PROP_PCR;
    UINT32 pulRespDataLength;
    BYTE* pNumPcrs;
    UINT32 hNVStorePublicInfoLength;
    BYTE* hNVStorePublicInfo = NULL;
    const char* thePassword = NULL;

    if ((a->owner_password == NULL) && (a->flags & TNV_FLAG_NEEDOWNER)) {
        TNV_stderr("TPM owner password is required for this operation.\n");
        result = TSP_ERROR(TSS_E_TSP_AUTHREQUIRED);
        goto out;
    }

    if ((a->password == NULL) && (a->flags & TNV_FLAG_RWAUTH)) {
        TNV_stderr("A password is required for this operation.\n");
        result = TSP_ERROR(TSS_E_TSP_AUTHREQUIRED);
        goto out;
    }

    t = calloc(1, sizeof(tnv_context_t));
    if (t == NULL) {
        return NULL;
    }

    result = Tspi_Context_Create(&t->hContext);
    if (result != TSS_SUCCESS) {
        TNV_syslog("Tspi_Context_Create", result);
        goto out;
    }

    if (tssServer != NULL) {
        wszDestination = TNV_utf8_to_utf16le((BYTE*)tssServer);
    }

    result = Tspi_Context_Connect(t->hContext, wszDestination);

    if (wszDestination != NULL) {
        free(wszDestination);
    }

    if (result != TSS_SUCCESS) {
        TNV_syslog("Tspi_Context_Connect", result);
        goto out;
    }

    result = Tspi_Context_GetTpmObject(t->hContext, &t->hTPM);
    if (result != TSS_SUCCESS) {
        TNV_syslog("Tspi_Context_GetTpmObject", result);
        goto out;
    }

    result = Tspi_Context_CreateObject(t->hContext,
                                       TSS_OBJECT_TYPE_POLICY,
                                       TSS_POLICY_USAGE,
                                       &t->hTPMUsagePolicy);
    if (result != TSS_SUCCESS) {
        TNV_syslog("Tspi_Context_CreateObject", result);
        goto out;
    }

    result = Tspi_Policy_AssignToObject(t->hTPMUsagePolicy, t->hTPM);
    if (result != TSS_SUCCESS) {
        TNV_syslog("Tspi_Policy_AssignToObject", result);
        goto out;
    }

    if (a->owner_password != NULL) {
        if (*(a->owner_password) == '\0') {
            result = Tspi_Policy_SetSecret(t->hTPMUsagePolicy,
                                           TSS_SECRET_MODE_SHA1,
                                           sizeof(well_known_secret),
                                           well_known_secret);
        } else {
            result = Tspi_Policy_SetSecret(t->hTPMUsagePolicy,
                                           TSS_SECRET_MODE_PLAIN,
                                           strlen(a->owner_password),
                                           (BYTE*)(a->owner_password));
        }
        if (result != TSS_SUCCESS) {
            TNV_syslog("Tspi_Policy_SetSecret", result);
            goto out;
        }
    }

    result = Tspi_TPM_GetCapability(t->hTPM,
                                    TSS_TPMCAP_PROPERTY,
                                    sizeof(UINT32),
                                    (BYTE*)&subCap,
                                    &pulRespDataLength,
                                    &pNumPcrs);
    if (result == TSS_SUCCESS) {
        t->numPcrs = *(UINT32*)pNumPcrs;
        Tspi_Context_FreeMemory(t->hContext, pNumPcrs);
    } else {
        TNV_syslog("Tspi_TPM_GetCapability", result);
        if (a->pcrs_selected.count == 0) {
            // Don't bail out; continue with t->numPcrs set to 0.
            result = TSS_SUCCESS;
        } else {
            goto out;
        }
    }

    if (a->flags & TNV_FLAG_NONSPECIFIC) {
        goto out;
    }

    result = Tspi_Context_CreateObject(t->hContext, TSS_OBJECT_TYPE_NV,
                                       0, &t->hNVStore);
    if (result != TSS_SUCCESS) {
        TNV_syslog("Tspi_Context_CreateObject", result);
        goto out;
    }

    result = Tspi_Context_CreateObject(t->hContext, TSS_OBJECT_TYPE_POLICY,
                                       TSS_POLICY_USAGE,
                                       &t->hNVStoreUsagePolicy);
    if (result != TSS_SUCCESS) {
        TNV_syslog("Tspi_Context_CreateObject", result);
        goto out;
    }

    if (a->flags & TNV_FLAG_RWAUTH) {
        thePassword = a->password;
    } else {
        thePassword = a->index_password;
    }

    if (thePassword != NULL) {
        if (*thePassword == '\0') {
            result = Tspi_Policy_SetSecret(t->hNVStoreUsagePolicy,
                                           TSS_SECRET_MODE_SHA1,
                                           sizeof(well_known_secret),
                                           well_known_secret);
        } else {
            result = Tspi_Policy_SetSecret(t->hNVStoreUsagePolicy,
                                           TSS_SECRET_MODE_PLAIN,
                                           strlen(thePassword),
                                           (BYTE*)thePassword);
        }
        if (result != TSS_SUCCESS) {
            TNV_syslog("Tspi_Policy_SetSecret", result);
            goto out;
        }
    } else {
        result = Tspi_Policy_SetSecret(t->hNVStoreUsagePolicy,
                                       TSS_SECRET_MODE_NONE, 0, NULL);
    }

    result = Tspi_Policy_AssignToObject(t->hNVStoreUsagePolicy, t->hNVStore);
    if (result != TSS_SUCCESS) {
        TNV_syslog("Tspi_Policy_AssignToObject", result);
        goto out;
    }

    result = Tspi_SetAttribUint32(t->hNVStore, TSS_TSPATTRIB_NV_INDEX,
                                  0, a->index);
    if (result != TSS_SUCCESS) {
        TNV_syslog("Tspi_SetAttribUint32", result);
        goto out;
    }

    if (!(a->flags & TNV_FLAG_CREATE) &&
        !(a->flags & TNV_FLAG_DESTROY)) {
        UINT32 targetIndex = a->index;
        result = Tspi_TPM_GetCapability(t->hTPM, TSS_TPMCAP_NV_INDEX,
                                        sizeof(UINT32), (BYTE*)&targetIndex,
                                        &hNVStorePublicInfoLength,
                                        &hNVStorePublicInfo);
        if (result != TSS_SUCCESS) {
            TNV_syslog("Tspi_TPM_GetCapability", result);
            goto out;
        }
        if (tnv_populate_nv_data_public(targetIndex, &t->hNVStorePublicInfo,
                                        hNVStorePublicInfoLength,
                                        hNVStorePublicInfo) != TRUE) {
            TNV_stderr("Failed to populate NV area public data.\n");
            result = TSP_ERROR(TSS_E_INTERNAL_ERROR);
            goto out;
        }

        t->hNVStorePublicInfo.initialized = TRUE;
   }

out:

    if (hNVStorePublicInfo != NULL) {
        (void)Tspi_Context_FreeMemory(t->hContext, hNVStorePublicInfo);
    }

    if (result != TSS_SUCCESS) {
        tnv_close_context(&t);
    }

    return t;
}

void
tnv_close_context(tnv_context_t** t)
{
    if (*t == NULL) {
        return;
    }

    if ((*t)->hContext != 0) {
        (void)Tspi_Context_CloseObject((*t)->hContext,
                                       (*t)->hNVStoreUsagePolicy);
        (void)Tspi_Context_CloseObject((*t)->hContext, (*t)->hNVStore);
        (void)Tspi_Context_CloseObject((*t)->hContext, (*t)->hTPMUsagePolicy);
        (void)Tspi_Context_CloseObject((*t)->hContext, (*t)->hTPM);
        (void)Tspi_Context_FreeMemory((*t)->hContext, NULL);
        (void)Tspi_Context_Close((*t)->hContext);
    }

    free(*t);
    *t = NULL;

    return;
}

int32_t
tnv_define(tnv_context_t* t, tnv_args_t* a)
{
    TSS_RESULT result;
    TSS_HPCRS hPcrCompositeRead = NULL_HOBJECT;
    TSS_HPCRS hPcrCompositeWrite = NULL_HOBJECT;
    BYTE* rgbPcrValue = NULL;
    UINT32 ulPcrLen;
    BYTE** pcrCache = NULL;
    UINT32 i;

    if (a->size == 0) {
        TNV_stderr("Cannot define a zero-sized space.\n");
        return TSP_ERROR(TSS_E_BAD_PARAMETER);
    }

    if (a->permissions != 0) {
        if ((a->permissions & TPM_NV_PER_AUTHREAD) &&
            (a->permissions & TPM_NV_PER_OWNERREAD)) {
            TNV_stderr("AUTHREAD and OWNERREAD are mutually exclusive.\n");
            return TSP_ERROR(TSS_E_BAD_PARAMETER);
        }
        if ((a->permissions & TPM_NV_PER_AUTHWRITE) &&
            (a->permissions & TPM_NV_PER_OWNERWRITE)) {
            TNV_stderr("AUTHWRITE and OWNERWRITE are mutually exclusive.\n");
            return TSP_ERROR(TSS_E_BAD_PARAMETER);
        }
        if (((a->permissions & TPM_NV_PER_OWNERREAD) ||
             (a->permissions & TPM_NV_PER_OWNERWRITE)) &&
             (a->owner_password == NULL)) {
            TNV_stderr("TPM owner password is required for this operation.\n");
            return TSP_ERROR(TSS_E_TSP_AUTHREQUIRED);
        }
        if (((a->permissions & TPM_NV_PER_AUTHREAD) ||
             (a->permissions & TPM_NV_PER_AUTHWRITE)) &&
             (a->index_password == NULL)) {
            TNV_stderr("Index password is required for this operation.\n");
            return TSP_ERROR(TSS_E_TSP_AUTHREQUIRED);
        }
    }

    if (a->pcrs_selected.count > 0) {

        TSS_FLAG initFlags = TSS_PCRS_STRUCT_INFO_SHORT; // 0;

        if (a->pcrs_selected.count >= t->numPcrs) {
            TNV_stderr("This TPM does not have a PCR with index %u.\n",
                       a->pcrs_selected.highest);
            TNV_stderr("The highest PCR index for this TPM is %u.\n",
                       t->numPcrs - 1);
            return TSP_ERROR(TSS_E_BAD_PARAMETER);
        }

        result = Tspi_Context_CreateObject(t->hContext, TSS_OBJECT_TYPE_PCRS,
                                           initFlags, &hPcrCompositeRead);
        if (result != TSS_SUCCESS) {
            TNV_syslog("Tspi_Context_CreateObject", result);
            return result;
        }

        result = Tspi_Context_CreateObject(t->hContext, TSS_OBJECT_TYPE_PCRS,
                                           initFlags, &hPcrCompositeWrite);
        if (result != TSS_SUCCESS) {
            TNV_syslog("Tspi_Context_CreateObject", result);
            return result;
        }

        pcrCache = (BYTE**)calloc(a->pcrs_selected.highest, sizeof(BYTE*));

        for (i = 0; i <= a->pcrs_selected.highest; i++) {
            if (!bit_test(a->pcrs_selected.bitmap, i)) {
                continue;
            }
            result = Tspi_TPM_PcrRead(t->hTPM, i, &ulPcrLen, &rgbPcrValue);
            if (result != TSS_SUCCESS) {
                TNV_syslog("Tspi_TPM_PcrRead", result);
                goto out;
            }
            pcrCache[i] = rgbPcrValue;
            result = Tspi_PcrComposite_SetPcrValue(hPcrCompositeRead, i,
                                                   ulPcrLen, rgbPcrValue);
            if (result != TSS_SUCCESS) {
                TNV_syslog("Tspi_PcrComposite_SetPcrValue", result);
                goto out;
            }
            if (ulPcrLen != SHA_DIGEST_LENGTH) {
                TNV_stderr("Unexpected length %u of PCR value (expected %u).\n",
                           ulPcrLen, SHA_DIGEST_LENGTH);
                goto out;
            }
            result = Tspi_PcrComposite_SetPcrValue(hPcrCompositeWrite, i,
                                                   ulPcrLen, rgbPcrValue);
            if (result != TSS_SUCCESS) {
                TNV_syslog("Tspi_PcrComposite_SetPcrValue", result);
                goto out;
            }
        }

        result = Tspi_PcrComposite_SetPcrLocality(hPcrCompositeRead,
                                                  TPM_LOC_ZERO  |
                                                  TPM_LOC_ONE   |
                                                  TPM_LOC_TWO   |
                                                  TPM_LOC_THREE |
                                                  TPM_LOC_FOUR);
        if (result != TSS_SUCCESS) {
            TNV_syslog("Tspi_PcrComposite_SetPcrLocality", result);
            goto out;
        }

        result = Tspi_PcrComposite_SetPcrLocality(hPcrCompositeWrite,
                                                  TPM_LOC_ZERO  |
                                                  TPM_LOC_ONE   |
                                                  TPM_LOC_TWO   |
                                                  TPM_LOC_THREE |
                                                  TPM_LOC_FOUR);
        if (result != TSS_SUCCESS) {
            TNV_syslog("Tspi_PcrComposite_SetPcrLocality", result);
            goto out;
        }
    }

    result = Tspi_SetAttribUint32(t->hNVStore, TSS_TSPATTRIB_NV_PERMISSIONS,
                                  0, a->permissions);
    if (result != TSS_SUCCESS) {
        TNV_syslog("Tspi_SetAttribUint32", result);
        goto out;
    }

    result = Tspi_SetAttribUint32(t->hNVStore, TSS_TSPATTRIB_NV_DATASIZE,
                                  0, a->size);
    if (result != TSS_SUCCESS) {
        TNV_syslog("Tspi_SetAttribUint32", result);
        goto out;
    }

    result = Tspi_NV_DefineSpace(t->hNVStore,
                                 hPcrCompositeRead, hPcrCompositeWrite);
    if (result != TSS_SUCCESS) {
        TNV_syslog("Tspi_NV_DefineSpace", result);
        goto out;
    }

out:
    if ((a->pcrs_selected.count > 0) && pcrCache) {
        for (i = 0; i <= a->pcrs_selected.highest; i++) {
            if (pcrCache[i] != NULL) {
                if (result == TSS_SUCCESS) {
                    UINT32 j;
                    TNV_stdout("# pcr%-2u = ", i);
                    for (j = 0; j < SHA_DIGEST_LENGTH; j++) {
                        TNV_stdout("%02x", pcrCache[i][j]);
                    }
                    TNV_stdout("\n");
                }
                (void)Tspi_Context_FreeMemory(t->hContext, pcrCache[i]);
            }
        }
        free(pcrCache);
    }

    if (hPcrCompositeRead != NULL_HOBJECT) {
        (void)Tspi_Context_CloseObject(t->hContext, hPcrCompositeRead);
    }

    if (hPcrCompositeWrite != NULL_HOBJECT) {
        (void)Tspi_Context_CloseObject(t->hContext, hPcrCompositeWrite);
    }

    return (result == TSS_SUCCESS) ? 0 : -1;
}

int32_t
tnv_list(tnv_context_t* t, tnv_args_t* a)
{
    TSS_RESULT result;
    UINT32 indexListLength;
    UINT32* indexList;
    UINT32 indexCount;
    UINT32 rawPublicInfoLength;
    BYTE* rawPublicInfo;
    UINT32 targetIndex;
    UINT32 i;

    if (!(a->flags & TNV_FLAG_NONSPECIFIC)) {
        indexCount = 1;
        targetIndex = OSSwapHostToBigInt32(a->index);
        indexList = &targetIndex;
    } else {
        result = Tspi_TPM_GetCapability(t->hTPM, TSS_TPMCAP_NV_LIST, 0, NULL,
                                        &indexListLength, (BYTE**)&indexList);
        if (result != TSS_SUCCESS) {
            TNV_syslog("Tspi_TPM_GetCapability", result);
            return -1;
        }
        indexCount = indexListLength / sizeof(UINT32);
    }

    for (i = 0; i < indexCount; i++) {
        targetIndex = OSSwapBigToHostInt32(indexList[i]);
        result = Tspi_TPM_GetCapability(t->hTPM, TSS_TPMCAP_NV_INDEX,
                                        sizeof(UINT32), (BYTE*)&targetIndex,
                                        &rawPublicInfoLength, &rawPublicInfo);
        if (result != TSS_SUCCESS) {
            TNV_syslog("Tspi_TPM_GetCapability", result);
        }
        tnv_print_nv_data_public(targetIndex, rawPublicInfoLength,
                                 rawPublicInfo);
        (void)Tspi_Context_FreeMemory(t->hContext, rawPublicInfo);
    }

    return (result == TSS_SUCCESS) ? 0 : -1;
}

int32_t
tnv_read(tnv_context_t* t, tnv_args_t* a)
{
    TSS_RESULT result;
    UINT32 readOffset;
    UINT32 readLength;
    BYTE* dataRead;
    tnv_data_public_t* pubInfo = &t->hNVStorePublicInfo;

    if (pubInfo->initialized != TRUE) {
        TNV_stderr("Failed to retrieve NV area public data.\n");
        return -1;
    }

    if (((a->offset + a->size) > pubInfo->dataSize) ||
        (a->offset == pubInfo->dataSize)) {
        TNV_stderr("Requested data lies beyond the NV area.\n");
        return -1;
    }

    readOffset = (a->offset != 0) ? a->offset : 0;
    readLength = (a->size != 0) ? a->size : (pubInfo->dataSize - a->offset);

    result = Tspi_NV_ReadValue(t->hNVStore, readOffset, &readLength, &dataRead);
    if (result != TSS_SUCCESS) {
        if (TSS_EAUTH(result) == TRUE) {
            TNV_stderr("Read: Invalid, incorrect, or missing password.\n");
        } else {
            TNV_syslog("Tspi_NV_ReadValue", result);
        }
        return -1;
    }

    if (!(a->flags & TNV_FLAG_HEXDUMP)) {
        (void)TNV_writeout(dataRead, readLength);
    } else {
        int i, j;
        int rows = readLength / 16;
        for (i = 0; i < rows; i++) {
            for (j = 0; j < 16; j++) {
                TNV_stdout("%02hhx ", dataRead[16 * i + j]);
            }
            TNV_stdout("\n");
        }
        j = readLength % 16; 
        if (j) {
            for (i = 0; i < j; i++) {
                TNV_stdout("%02hhx ", dataRead[16 * rows + i]);
            }
            TNV_stdout("\n");
        }
    }

    (void)Tspi_Context_FreeMemory(t->hContext, dataRead);

    return (result == TSS_SUCCESS) ? 0 : -1;
}

int32_t
tnv_release(tnv_context_t* t, tnv_args_t* a)
{
    TSS_RESULT result;

    result = Tspi_NV_ReleaseSpace(t->hNVStore);
    if (result != TSS_SUCCESS) {
        TNV_syslog("Tspi_NV_ReleaseSpace", result);
        return -1;
    }

    return 0;
}

int32_t
tnv_write(tnv_context_t* t, tnv_args_t* a)
{
    TSS_RESULT result;
    UINT32 writeOffset;
    UINT32 writeLength;
    tnv_data_public_t* pubInfo = &t->hNVStorePublicInfo;

    if (pubInfo->initialized != TRUE) {
        TNV_stderr("Failed to retrieve NV area public data.\n");
        return -1;
    }

    if ((a->offset == 0) && (a->size == 0)) { // writezero
        result = Tspi_NV_WriteValue(t->hNVStore, 0, 0, NULL);
    } else {
        if (((a->offset + a->size) > pubInfo->dataSize) ||
            (a->offset == pubInfo->dataSize)) {
            TNV_stderr("Requested data destination lies beyond the NV area.\n");
            return -1;
        }
        writeOffset = (a->offset != 0) ? a->offset : 0;
        writeLength =
            (a->size != 0) ? a->size : (pubInfo->dataSize - a->offset);

        result = Tspi_NV_WriteValue(t->hNVStore, writeOffset, writeLength,
                                    (BYTE*)(a->data));
    }

    if (result != TSS_SUCCESS) {
        if (TSS_EAUTH(result) == TRUE) {
            TNV_stderr("Write: Invalid, incorrect, or missing password.\n");
        } else {
            TNV_syslog("Tspi_NV_WriteValue", result);
        }
        return -1;
    }

    return 0;
}
