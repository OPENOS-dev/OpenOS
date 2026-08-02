// Copyright 2021 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TSS_TSS_H_
#define TSS_TSS_H_

#include "tss_types.h"
#include "authorization_delegate.h"

// Use the TPM_RC type but with different layer bits (12 - 15). Choose the layer
// value arbitrarily.
#define TSS_ERROR_BASE ((TPM_RC)(7 << 12))

#define TSS_RC_AUTHORIZATION_FAILED ((TPM_RC)(TSS_ERROR_BASE + 1))
#define TSS_RC_ENCRYPTION_FAILED ((TPM_RC)(TSS_ERROR_BASE + 2))
#define TSS_RC_READ_ERROR ((TPM_RC)(TSS_ERROR_BASE + 3))
#define TSS_RC_WRITE_ERROR ((TPM_RC)(TSS_ERROR_BASE + 4))
#define TSS_RC_IPC_ERROR ((TPM_RC)(TSS_ERROR_BASE + 5))
#define TSS_RC_SESSION_SETUP_ERROR ((TPM_RC)(TSS_ERROR_BASE + 6))
#define TSS_RC_INVALID_TPM_CONFIGURATION ((TPM_RC)(TSS_ERROR_BASE + 7))
#define TSS_RC_SERIALIZATION_ERROR ((TPM_RC)(TSS_ERROR_BASE + 8))
#define TSS_RC_HASH_ERROR ((TPM_RC)(TSS_ERROR_BASE + 9))

TPM_RC tss_StartAuthSession(
    const TPMI_DH_OBJECT tpm_key,
    const TPM2B_NAME* tpm_key_name,
    const TPMI_DH_ENTITY bind,
    const TPM2B_NAME* bind_name,
    const TPM2B_NONCE* nonce_caller,
    const TPM2B_ENCRYPTED_SECRET* encrypted_salt,
    const TPM_SE session_type,
    const TPMT_SYM_DEF* symmetric,
    const TPMI_ALG_HASH auth_hash,
    TPMI_SH_AUTH_SESSION* session_handle,
    TPM2B_NONCE* nonce_tpm,
    TSS_AUTHORIZATION_DELEGATE* authorization_delegate);

TPM_RC tss_ReadPublic(
    const TPMI_DH_OBJECT object_handle,
    const TPM2B_NAME* object_handle_name,
    TPM2B_PUBLIC* out_public,
    TPM2B_NAME* name,
    TPM2B_NAME* qualified_name,
    TSS_AUTHORIZATION_DELEGATE* authorization_delegate);

TPM_RC tss_PolicyCommandCode(
    const TPMI_SH_POLICY policy_session,
    const TPM2B_NAME* policy_session_name,
    const TPM_CC code,
    TSS_AUTHORIZATION_DELEGATE* authorization_delegate);

TPM_RC tss_PolicyAuthValue(
    const TPMI_SH_POLICY policy_session,
    const TPM2B_NAME* policy_session_name,
    TSS_AUTHORIZATION_DELEGATE* authorization_delegate);

TPM_RC tss_FlushContext(
    const TPMI_DH_CONTEXT flush_handle,
    TSS_AUTHORIZATION_DELEGATE* authorization_delegate);

TPM_RC tss_NV_DefineSpace(
    const TPMI_RH_PROVISION auth_handle,
    const TPM2B_NAME* auth_handle_name,
    const TPM2B_AUTH* auth,
    const TPM2B_NV_PUBLIC* public_info,
    TSS_AUTHORIZATION_DELEGATE* authorization_delegate);

TPM_RC tss_NV_ReadPublic(
    const TPMI_RH_NV_INDEX nv_index,
    const TPM2B_NAME* nv_index_name,
    TPM2B_NV_PUBLIC* nv_public,
    TPM2B_NAME* nv_name,
    TSS_AUTHORIZATION_DELEGATE* authorization_delegate);

TPM_RC tss_NV_Write(
    const TPMI_RH_NV_AUTH auth_handle,
    const TPM2B_NAME* auth_handle_name,
    const TPMI_RH_NV_INDEX nv_index,
    const TPM2B_NAME* nv_index_name,
    const TPM2B_MAX_NV_BUFFER* data,
    const UINT16 offset,
    TSS_AUTHORIZATION_DELEGATE* authorization_delegate);

TPM_RC tss_NV_Read(
    const TPMI_RH_NV_AUTH auth_handle,
    const TPM2B_NAME* auth_handle_name,
    const TPMI_RH_NV_INDEX nv_index,
    const TPM2B_NAME* nv_index_name,
    const UINT16 size,
    const UINT16 offset,
    TPM2B_MAX_NV_BUFFER* data,
    TSS_AUTHORIZATION_DELEGATE* authorization_delegate);

TPM_RC tss_NV_ChangeAuth(
    const TPMI_RH_NV_INDEX nv_index,
    const TPM2B_NAME* nv_index_name,
    const TPM2B_AUTH* new_auth,
    TSS_AUTHORIZATION_DELEGATE* authorization_delegate);

#endif  // TSS_TSS_H_
