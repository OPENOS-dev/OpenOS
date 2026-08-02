// Copyright (c) 2009 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <tpm_keychain_common.h>

#include <netinet/in.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>

typedef struct {
    TSS_BOOL     hInitialized;
    TSS_HCONTEXT hContext;
    TSS_HKEY     hSRK;
    TSS_HKEY     hKCHead;
    TSS_HKEY     hKey;
    TSS_HPOLICY  hSRKUsagePolicy;
    TSS_HPOLICY  hKCHeadUsagePolicy;
    TSS_HPOLICY  hKeyUsagePolicy;
    UINT32       hSRKUsageAuth;
    UINT32       hKCHeadUsageAuth;
    UINT32       hKeyUsageAuth;
} tpm_ssh_key_context_t;

static tpm_ssh_key_context_t tsk = { FALSE, 0, };
static const TSS_UUID KEYCHAIN_HEAD_UUID = TKC_HEAD_UUID;

static int tpm_ssh_read_uuid(FILE* fp, uuid_t* uuid);

#ifdef __APPLE__

EVP_PKEY* TPM_SSH_PEM_read_PrivateKey(FILE* fp, EVP_PKEY** x,
                                      pem_password_cb* cb, void* u);

int TPM_SSH_RSA_sign(int type, const unsigned char* m, unsigned int m_len,
                     unsigned char* sigret, unsigned int* siglen, RSA* rsa);

typedef struct interpose_s {
    void* new_func;
    void* old_func;
} interpose_t;

static const interpose_t interposers[]
    __attribute__((section("__DATA, __interpose"), used)) = {
    { (void*)TPM_SSH_PEM_read_PrivateKey, (void*)PEM_read_PrivateKey },
    { (void*)TPM_SSH_RSA_sign, (void*)RSA_sign },
};

#else

#define _GNU_SOURCE
#define __USE_GNU
#include <dlfcn.h>

#define TPM_SSH_PEM_read_PrivateKey PEM_read_PrivateKey
#define TPM_SSH_RSA_sign RSA_sign

#endif

EVP_PKEY*
TPM_SSH_PEM_read_PrivateKey(FILE* fp, EVP_PKEY** x,
                            pem_password_cb* cb, void* u)
{
    TSS_RESULT result = TSP_ERROR(TSS_E_INTERNAL_ERROR);
    TSS_UUID SRK_UUID = TSS_UUID_SRK;
    uuid_t tpm_ssh_uuid;
    UINT32 authRetryCounter = 0;
    tpm_ssh_key_context_t* t = &tsk;

    char* tss_server = getenv(TKC_SERVER_ENVIRONMENT_VARIABLE);
    TSS_UNICODE* wszDestination = NULL;

    BYTE* blob_e = NULL;
    BYTE* blob_n = NULL;
    UINT32 blobLen_e;
    UINT32 blobLen_n;
    BIGNUM* e = NULL;
    BIGNUM* n = NULL;
    EVP_PKEY* epk = NULL;
    RSA* rsa = NULL;

    if (tpm_ssh_read_uuid(fp, &tpm_ssh_uuid) != 0) {
#ifdef __APPLE__
        return PEM_read_PrivateKey(fp, x, cb, u);
#else
        typedef EVP_PKEY* (*PEM_read_PrivateKey_t)(FILE*, EVP_PKEY**,
                                                   pem_password_cb*, void*);
        PEM_read_PrivateKey_t lib_PEM_read_PrivateKey =
                                  dlsym(RTLD_NEXT, "PEM_read_PrivateKey");
        if (lib_PEM_read_PrivateKey == NULL) {
            return NULL;
        }
        return lib_PEM_read_PrivateKey(fp, x, cb, u);
#endif
    }

    result = Tspi_Context_Create(&t->hContext);
    if (result != TSS_SUCCESS) {
        TKC_syslog("Tspi_Context_Create", result);
        goto out;
    }

    if (tss_server != NULL) {
        wszDestination = TKC_utf8_to_utf16le((BYTE*)tss_server);
    }

    result = Tspi_Context_Connect(t->hContext, wszDestination);

    if (wszDestination != NULL) {
        free(wszDestination);
    }

    if (result != TSS_SUCCESS) {
        TKC_syslog("Tspi_Context_Connect", result);
        goto out;
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
    if (result != TSS_SUCCESS) {
        TKC_syslog("Tspi_Context_GetKeyByUUID", result);
        goto out;
    }

    authRetryCounter = 0;

    while (1) {
        if (TKC_auth_should_retry(t->hSRKUsageAuth, t->hSRKUsagePolicy,
                                  NULL, SRK_AUTH_RETRY_ID, result, 
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

    result = Tspi_Context_GetKeyByUUID(t->hContext, TSS_PS_TYPE_USER,
                                       *(TSS_UUID*)&tpm_ssh_uuid, &t->hKey);
    if (result != TSS_SUCCESS) {
        TKC_syslog("Tspi_Context_GetKeyByUUID", result);
        goto out;
    }

    authRetryCounter = 0;

    while (1) {
        if (TKC_auth_should_retry(t->hKCHeadUsageAuth, t->hKCHeadUsagePolicy,
                                  NULL, KEYCHAIN_AUTH_RETRY_ID, result,
                                  &authRetryCounter) == TRUE) {
            result = Tspi_Key_LoadKey(t->hKey, t->hKCHead);
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

    result = TKC_auth_init_keyusage_policy(t->hContext, t->hKey,
                                           &t->hKeyUsagePolicy,
                                           &t->hKeyUsageAuth);
    if (result != TSS_SUCCESS) {
        goto out;
    }

    result = Tspi_GetAttribData(t->hKey, TSS_TSPATTRIB_RSAKEY_INFO,
                                TSS_TSPATTRIB_KEYINFO_RSA_EXPONENT,
                                &blobLen_e, &blob_e);
    if (result != TSS_SUCCESS) {
        TKC_syslog("Tspi_GetAttribData", result);
        goto out;
    }

    result = Tspi_GetAttribData(t->hKey, TSS_TSPATTRIB_RSAKEY_INFO,
                                TSS_TSPATTRIB_KEYINFO_RSA_MODULUS,
                                &blobLen_n, &blob_n);
    if (result != TSS_SUCCESS) {
        TKC_syslog("Tspi_GetAttribData", result);
        goto out;
    }

    e = BN_new();
    if (e == NULL) {
        TKC_stderr("Failed to allocate memory.\n");
        goto out;
    }

    n = BN_new();
    if (n == NULL) {
        TKC_stderr("Failed to allocate memory.\n");
        goto out;
    }

    rsa = RSA_new();
    if (rsa == NULL) {
        TKC_stderr("Failed to allocate memory.\n");
        goto out;
    }

    BN_bin2bn(blob_e, blobLen_e, e);
    BN_bin2bn(blob_n, blobLen_n, n);
    rsa->e = e;
    rsa->n = n;

    epk = EVP_PKEY_new();
    if (epk == NULL) {
        TKC_stderr("Failed to allocate memory.\n");
        goto out;
    }

    if (EVP_PKEY_set1_RSA(epk, rsa) != 1) {
        TKC_stderr("Failed to initialize key.\n");
        goto out;
    }

    t->hInitialized = TRUE;

out:
    if (blob_e != NULL) {
        (void)Tspi_Context_FreeMemory(t->hContext, blob_e);
    }
    if (blob_n != NULL) {
        (void)Tspi_Context_FreeMemory(t->hContext, blob_n);
    }
    if (t->hInitialized == FALSE) {
        if (t->hContext != 0) {
            (void)Tspi_Context_FreeMemory(t->hContext, NULL);
            (void)Tspi_Context_Close(t->hContext);
            t->hContext = 0;
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

        if (epk) {
            EVP_PKEY_free(epk);
        }

        return NULL;
    }

    return epk;
}

int
TPM_SSH_RSA_sign(int type, const unsigned char* m, unsigned int m_len,
                 unsigned char* sigret, unsigned int* siglen, RSA* rsa)
{
    int ret = 0;
    tpm_ssh_key_context_t* t = &tsk;
    TSS_RESULT result;
    TSS_HHASH hHash = NULL_HOBJECT;
    BYTE* localSig = NULL;
    UINT32 localSigLen;
    UINT32 authRetryCounter = 0;

    if (t->hInitialized == FALSE) {
#ifdef __APPLE__
        return RSA_sign(type, m, m_len, sigret, siglen, rsa);
#else
        typedef int (*RSA_sign_t)(int, const unsigned char*, unsigned int,
                                  unsigned char*, unsigned int*, RSA*);
        RSA_sign_t lib_RSA_sign = dlsym(RTLD_NEXT, "RSA_sign");
        if (lib_RSA_sign == NULL) {
            return 0;
        }
        return lib_RSA_sign(type, m, m_len, sigret, siglen, rsa);
#endif
    }

    result = Tspi_Context_CreateObject(t->hContext, TSS_OBJECT_TYPE_HASH,
                                       TSS_HASH_SHA1, &hHash);
    if (result != TSS_SUCCESS) {
        TKC_syslog("Tspi_Context_CreateObject", result);
        goto out;
    }

    result = Tspi_Hash_SetHashValue(hHash, m_len, (BYTE*)m);
    if (result != TSS_SUCCESS) {
        TKC_syslog("Tspi_Hash_SetHashValue", result);
        goto out;
    }

    while (1) {
        if (TKC_auth_should_retry(t->hKeyUsageAuth, t->hKeyUsagePolicy,
                                  NULL, KEY_AUTH_RETRY_ID, result,
                                  &authRetryCounter) == TRUE) {
            result = Tspi_Hash_Sign(hHash, t->hKey, &localSigLen, &localSig);
            if (result == TSS_SUCCESS) {
                break;
            }
        } else {
            TKC_syslog("Tspi_Hash_Sign", result);
            goto out;
        }
    }

    if (result != TSS_SUCCESS) {
        goto out;
    }

    result = Tspi_Hash_VerifySignature(hHash, t->hKey, localSigLen, localSig);
    if (result != TSS_SUCCESS) {
        TKC_syslog("Tspi_Hash_VerifySignature", result);
        goto out;
    }

    memcpy(sigret, localSig, localSigLen);
    *siglen = localSigLen;

    ret = 1;

out:
    if (hHash != NULL_HOBJECT) {
        (void)Tspi_Context_CloseObject(t->hContext, hHash);
    }

    if (localSig != NULL) {
        (void)Tspi_Context_FreeMemory(t->hContext, localSig);
    }

    return ret;
}

static int
get_next_line(FILE* fp, char* buf, size_t bufsize)
{
    while (fgets(buf, bufsize, fp) != NULL) {
        if (buf[0] == '\0') {
            continue;
        }
        if ((buf[strlen(buf) - 1] == '\n') || feof(fp)) {
            return 0;
        } else {
            while ((fgetc(fp) != '\n') && !feof(fp)) {
                ;
            }
        }
    }

    return -1;
}

static int
tpm_ssh_read_uuid(FILE* fp, uuid_t* uuid)
{
    char line[8192];
    char* comment;
    char* cp;

    while (get_next_line(fp, line, sizeof(line)) != -1) {
        for (cp = line; *cp; cp++) {
            if ((*cp == '\n') || (*cp == ' ') || (*cp == '\t')) {
                continue;
            }
            comment = strchr(cp, '#');
            if (comment == NULL) {
                break;
            }
            cp = comment + 1;
            if (*cp) {
                char** ap;
                char*  av[2] = { NULL, NULL };
                for (ap = av; (*ap = strsep(&cp, " \t\n")) != NULL;) {
                    if (**ap != '\0') {
                        if (++ap >= &av[2]) {
                            break;
                        }
                    }
                }
                if ((av[0] == NULL) || (av[1] == NULL)) {
                    break;
                }
                if (strcmp(TKC_SSH_UUID_TAG, av[0]) != 0) {
                    break;
                }

                return uuid_parse(av[1], *uuid);
            }
        }
    }

    return -1;
}
