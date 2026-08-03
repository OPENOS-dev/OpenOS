// Copyright 2021 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>

#include "mock_tss.h"

using testing::_;
using testing::Invoke;
using testing::NiceMock;
using testing::Return;

std::unique_ptr<TssInterface> g_Tss;
TssInterface *g_TssPtr = nullptr;

TssInterface *GetTss() {
  if (!g_TssPtr) {
    if (!g_Tss) {
      g_Tss.reset(new NiceMock<MockTssInterface>());
    }
    g_TssPtr = g_Tss.get();
  }
  return g_TssPtr;
}
void SetTss(TssInterface *interface) { g_TssPtr = interface; }

MockTssInterface::MockTssInterface() {
  ON_CALL(*this, StartAuthSession(_, _, _, _))
      .WillByDefault(Return(TPM_RC_SUCCESS));
  ON_CALL(*this, ReadPublic(_, _, _, _, _, _))
      .WillByDefault(Return(TPM_RC_SUCCESS));
  ON_CALL(*this, PolicyCommandCode(_, _, _, _))
      .WillByDefault(Return(TPM_RC_SUCCESS));
  ON_CALL(*this, PolicyAuthValue(_, _, _))
      .WillByDefault(Return(TPM_RC_SUCCESS));
  ON_CALL(*this, FlushContext(_, _)).WillByDefault(Return(TPM_RC_SUCCESS));
  ON_CALL(*this, NV_DefineSpace(_, _, _, _, _))
      .WillByDefault(Return(TPM_RC_SUCCESS));
  ON_CALL(*this, NV_ReadPublic(_, _, _, _, _))
      .WillByDefault(Return(TPM_RC_SUCCESS));
  ON_CALL(*this, NV_Write(_, _, _, _, _, _, _))
      .WillByDefault(Return(TPM_RC_SUCCESS));
  ON_CALL(*this, NV_Read(_, _, _, _, _, _, _, _))
      .WillByDefault(Return(TPM_RC_SUCCESS));
  ON_CALL(*this, NV_ChangeAuth(_, _, _, _))
      .WillByDefault(Return(TPM_RC_SUCCESS));
  ON_CALL(*this, InitSessionHmac(_, _, _, _, _, _, _)).WillByDefault(Return(0));
  ON_CALL(*this, Serialize_TPMS_NV_PUBLIC(_, _))
      .WillByDefault(Return(TPM_RC_SUCCESS));
  ON_CALL(*this, Serialize_TPMS_ECC_POINT(_, _))
      .WillByDefault(Return(TPM_RC_SUCCESS));
}

extern "C" {

TPM_RC tss_StartAuthSession(
    const TPMI_DH_OBJECT tpm_key, const TPM2B_NAME *tpm_key_name,
    const TPMI_DH_ENTITY bind, const TPM2B_NAME *bind_name,
    const TPM2B_NONCE *nonce_caller,
    const TPM2B_ENCRYPTED_SECRET *encrypted_salt, const TPM_SE session_type,
    const TPMT_SYM_DEF *symmetric, const TPMI_ALG_HASH auth_hash,
    TPMI_SH_AUTH_SESSION *session_handle, TPM2B_NONCE *nonce_tpm,
    TSS_AUTHORIZATION_DELEGATE *authorization_delegate) {
  const MockTssInterface::StartAuthSessionParam param = {
      tpm_key,        tpm_key_name, bind,      bind_name, nonce_caller,
      encrypted_salt, session_type, symmetric, auth_hash};
  return GetTss()->StartAuthSession(param, session_handle, nonce_tpm,
                                    authorization_delegate);
}

TPM_RC tss_ReadPublic(const TPMI_DH_OBJECT object_handle,
                      const TPM2B_NAME *object_handle_name,
                      TPM2B_PUBLIC *out_public, TPM2B_NAME *name,
                      TPM2B_NAME *qualified_name,
                      TSS_AUTHORIZATION_DELEGATE *authorization_delegate) {
  return GetTss()->ReadPublic(object_handle, object_handle_name, out_public,
                              name, qualified_name, authorization_delegate);
}

TPM_RC
tss_PolicyCommandCode(const TPMI_SH_POLICY policy_session,
                      const TPM2B_NAME *policy_session_name, const TPM_CC code,
                      TSS_AUTHORIZATION_DELEGATE *authorization_delegate) {
  return GetTss()->PolicyCommandCode(policy_session, policy_session_name, code,
                                     authorization_delegate);
}

TPM_RC tss_PolicyAuthValue(const TPMI_SH_POLICY policy_session,
                           const TPM2B_NAME *policy_session_name,
                           TSS_AUTHORIZATION_DELEGATE *authorization_delegate) {
  return GetTss()->PolicyAuthValue(policy_session, policy_session_name,
                                   authorization_delegate);
}

TPM_RC tss_FlushContext(const TPMI_DH_CONTEXT flush_handle,
                        TSS_AUTHORIZATION_DELEGATE *authorization_delegate) {
  return GetTss()->FlushContext(flush_handle, authorization_delegate);
}

TPM_RC tss_NV_DefineSpace(const TPMI_RH_PROVISION auth_handle,
                          const TPM2B_NAME *auth_handle_name,
                          const TPM2B_AUTH *auth,
                          const TPM2B_NV_PUBLIC *public_info,
                          TSS_AUTHORIZATION_DELEGATE *authorization_delegate) {
  return GetTss()->NV_DefineSpace(auth_handle, auth_handle_name, auth,
                                  public_info, authorization_delegate);
}

TPM_RC tss_NV_ReadPublic(const TPMI_RH_NV_INDEX nv_index,
                         const TPM2B_NAME *nv_index_name,
                         TPM2B_NV_PUBLIC *nv_public, TPM2B_NAME *nv_name,
                         TSS_AUTHORIZATION_DELEGATE *authorization_delegate) {
  return GetTss()->NV_ReadPublic(nv_index, nv_index_name, nv_public, nv_name,
                                 authorization_delegate);
}

TPM_RC tss_NV_Write(const TPMI_RH_NV_AUTH auth_handle,
                    const TPM2B_NAME *auth_handle_name,
                    const TPMI_RH_NV_INDEX nv_index,
                    const TPM2B_NAME *nv_index_name,
                    const TPM2B_MAX_NV_BUFFER *data, const UINT16 offset,
                    TSS_AUTHORIZATION_DELEGATE *authorization_delegate) {
  return GetTss()->NV_Write(auth_handle, auth_handle_name, nv_index,
                            nv_index_name, data, offset,
                            authorization_delegate);
}

TPM_RC tss_NV_Read(const TPMI_RH_NV_AUTH auth_handle,
                   const TPM2B_NAME *auth_handle_name,
                   const TPMI_RH_NV_INDEX nv_index,
                   const TPM2B_NAME *nv_index_name, const UINT16 size,
                   const UINT16 offset, TPM2B_MAX_NV_BUFFER *data,
                   TSS_AUTHORIZATION_DELEGATE *authorization_delegate) {
  return GetTss()->NV_Read(auth_handle, auth_handle_name, nv_index,
                           nv_index_name, size, offset, data,
                           authorization_delegate);
}

TPM_RC tss_NV_ChangeAuth(const TPMI_RH_NV_INDEX nv_index,
                         const TPM2B_NAME *nv_index_name,
                         const TPM2B_AUTH *new_auth,
                         TSS_AUTHORIZATION_DELEGATE *authorization_delegate) {
  return GetTss()->NV_ChangeAuth(nv_index, nv_index_name, new_auth,
                                 authorization_delegate);
}

int tss_InitSessionHmac(TSS_AUTHORIZATION_DELEGATE_HMAC *delegate,
                        TPM_HANDLE session_handle, const TPM2B_NONCE *tpm_nonce,
                        const TPM2B_NONCE *caller_nonce,
                        const TPM2B_DIGEST *salt,
                        const TPM2B_DIGEST *bind_auth_value,
                        bool enable_parameter_encryption) {
  return GetTss()->InitSessionHmac(delegate, session_handle, tpm_nonce,
                                   caller_nonce, salt, bind_auth_value,
                                   enable_parameter_encryption);
}

TPM_RC tss_Serialize_TPMS_NV_PUBLIC(const TPMS_NV_PUBLIC *value,
                                    TSS_DST_DATA_BUF *buffer) {
  return GetTss()->Serialize_TPMS_NV_PUBLIC(value, buffer);
}

TPM_RC tss_Serialize_TPMS_ECC_POINT(const TPMS_ECC_POINT *value,
                                    TSS_DST_DATA_BUF *buffer) {
  return GetTss()->Serialize_TPMS_ECC_POINT(value, buffer);
}

} // extern "C"
